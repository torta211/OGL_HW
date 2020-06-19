#version 330 core

// variables coming from the pipeline
in vec3 vs_out_pos;

out vec4 fs_out_col;

// external parameters of the shader
uniform vec3 cam_pos;
uniform vec3 cam_forward;
uniform vec3 cam_up;
uniform vec3 cam_right;
uniform float screen_width;
uniform float screen_height;
uniform uint time;

// "constants"
uniform float tanHalfFieldOfView = 1.0;
const float SQRT2 = sqrt(2);
const float PI = 3.14159265;
const int traceMaxIter = 256;
const float rayMaxDist = 500.0;
const float rayMinDist = 0.01;
const float coneStartRadius = 0.01;
const float epsForHitAcceptance = 0.001;
const float epsForShadows = 0.1;
const float epsForNormal = 0.001;

// _-_-__-_-__-_-__-_-__-_-_ RAYMARCHING _-_-__-_-__-_-__-_-__-_-_
// primitives
// sphere with center at c, radius of s
float sdSphere(in vec3 p, in float r, in vec3 c)
{
	return length(p - c) - r;
}
// box with side lengths of b positioned at c
float sdBox(in vec3 p, in vec3 b, in vec3 c)
{
	vec3 d = abs(p - c) - b;
	return length(max(d, 0.0)) + min(max(d.x, max(d.y, d.z)), 0.0);
}
// describing world
const vec3 lightPos = vec3(5.0, 5.0, 5.0);
const float lightSourceRadius = 0.3;
float sdf(const in vec3 p)
{    
	vec3 z = p;
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < 36 ; i++)
    {
        r = length(z);
		if (r > 10.0) break;
		
		// convert to polar coordinates
		float theta = acos(z.z/r);
		float phi = atan(z.y,z.x);
		dr =  pow( r, 8.0 - 1.0) * 8.0 * dr + 1.0;
		
		// scale and rotate the point
		float zr = pow(r, 8.0);
		theta = theta * 8.0;
		phi = phi * 8.0;
		
		// convert back to cartesian coordinates
		z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		z += p;
	}
	return 0.5 * log(r) * r / dr;
}

vec3 normal(const in vec3 p)
{
    vec3 plus = vec3(sdf(p + vec3(epsForNormal, 0.0, 0.0)),
                     sdf(p + vec3(0.0, epsForNormal, 0.0)),
                     sdf(p + vec3(0.0, 0.0, epsForNormal)));
    vec3 minu = vec3(sdf(p - vec3(epsForNormal, 0.0, 0.0)),
                     sdf(p - vec3(0.0, epsForNormal, 0.0)),
                     sdf(p - vec3(0.0, 0.0, epsForNormal)));
    return normalize(plus - minu);
}

// struct for the ray
struct Ray
{
	vec3 start;
	vec3 dir;
    float tanHalfAngle;
    float distTaken;// Distance taken on ray
    float maxDist;	// Maximum distance allowed for this ray
    int flags;      // bit 0:   surface condition:      true if distance to surface is small < error threshold
    				// bit 1:   distance condition:     true if travelled to far t > t_max
                	// bit 2:   iteration condition:	true if took too many iterations
    				// bit 3: 	analytic condition:		true if we found an analytical solution for this ray (not used)
};
    
// cone tracing algorithms
void traceRayCamToPx(inout Ray ray)
{
    float distanceFromAxisToAnything, distanceFromConeToAnything;
    int i = 0;
    while (ray.flags == 0)
    {
        distanceFromAxisToAnything = sdf(ray.start + ray.distTaken * ray.dir);
        distanceFromConeToAnything = distanceFromAxisToAnything - (coneStartRadius + ray.distTaken * ray.tanHalfAngle);
        ray.distTaken += distanceFromConeToAnything / (1.0 + abs(ray.tanHalfAngle));
        ray.flags |= int(distanceFromConeToAnything <= epsForHitAcceptance);
        ray.flags |= (int(ray.distTaken >= ray.maxDist) << 1);
        ray.flags |= (int(++i >= traceMaxIter) << 2);
    }
    return;
}    
float traceRayPointToLight(inout Ray ray)
{    
    float distanceFromAxisToAnything, distanceFromConeToAnything;
    float m = 1.0;
    float currentTanHalfAngle = ray.tanHalfAngle;
    float currentStartRadius = coneStartRadius;
    int i = 0;
    while (ray.flags == 0)
    {
        distanceFromAxisToAnything = sdf(ray.start + ray.distTaken * ray.dir);
        distanceFromConeToAnything = distanceFromAxisToAnything - (currentStartRadius + ray.distTaken * currentTanHalfAngle);
        // here we use the original angle, since out error allowance only depends on the distance
        if (distanceFromConeToAnything < epsForShadows * ray.distTaken * ray.tanHalfAngle)
        {
            m = distanceFromAxisToAnything / (coneStartRadius + ray.distTaken * ray.tanHalfAngle);
            currentStartRadius = (m - epsForShadows) * coneStartRadius;
            currentTanHalfAngle = (m - epsForShadows) * ray.tanHalfAngle;
        }
        ray.distTaken += distanceFromConeToAnything / (1.0 + abs(currentTanHalfAngle));
        ++i;        
        // we do not set hit flag here, since it won't be used
        ray.flags |= (int(ray.distTaken >= ray.maxDist) << 1);
        ray.flags |= (int(i >= traceMaxIter) << 2);
        // but we use a flag for monitoring m
        ray.flags |= (int(m <= -1.0) << 4);
    }    
    return m;
}

// coloring (normals, shadowing, etc.)
vec4 hitColor(Ray ray)
{
    vec3 hitPoint = ray.start + ray.dir * ray.distTaken;
    vec3 hitNorm = normal(hitPoint);
    
    vec3 hitToLight = normalize(lightPos - hitPoint);
    float lightDist = distance(hitPoint, lightPos);
    
     //diffuse
    float costheta = max(dot(hitNorm, hitToLight), 0.0);
    vec3 k_d = vec3(1.0 / PI);
    
    //specular
    vec3 origToCam = -1.0 * ray.start;
    vec3 k_s = vec3(0.8);
    
    vec3 h = normalize(hitToLight + origToCam);
    float si = pow(clamp(dot(h, hitNorm), 0.0, 1.0), 160.0);
    
    vec3 col = (k_d + si * k_s) * costheta;
    
	Ray ray2 = Ray(hitPoint,						// starting from the point where the ray hit
                   hitToLight,						// going towards the light source
                   lightSourceRadius / lightDist,	// the halfangle at which we see the light
                   rayMinDist,						// starting distance
                   lightDist,						// we go til the light source at maximum
                   0);								// all flags are zero
    // we sacle the original m from -1 to 1, to -PI/2 to PI/2
    float m = traceRayPointToLight(ray2) * PI / 2.0;
    // estimating the area of the cut circle
    col *= (sin(m) + 1.0) / 2.0;
    
    return vec4(col, 1.0);
}

void main()
{
	vec2 px = vs_out_pos.xy * normalize(vec2(screen_width, screen_height)) * tanHalfFieldOfView;
	vec3 ray_dir = normalize(cam_forward + cam_right * px.x + cam_up * px.y);
	float tanHalfPixelAngle = sqrt(2.0) * tanHalfFieldOfView / screen_width;
	Ray rayToPx = Ray(cam_pos, ray_dir, tanHalfPixelAngle, rayMinDist, rayMaxDist, 0);
    traceRayCamToPx(rayToPx);     
    // if we would use analytic calculations, we would have to check that first
    if (bool(rayToPx.flags & 1))
    {
        // case for actually hitting something
        fs_out_col = hitColor(rayToPx);
        gl_FragDepth = -1.0;
    }
    else if(bool(rayToPx.flags & 2) || bool(rayToPx.flags & 4))
    {
        // case for getting too far, or getting to too many iterations
        fs_out_col = vec4(0.0);
        gl_FragDepth = 100.0;
    }
}