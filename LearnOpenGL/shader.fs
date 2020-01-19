#version 330 core
#define MAXFLOAT 10000000.0
#define M_PI 3.1415926
precision highp float;
out vec4 fragColor;
in vec2 fragCoord;
in vec2 texCoord;
uniform vec3 iResolution;
uniform vec4 iMouse;
uniform float iTime;
uniform int RandSeed;
uniform sampler2D ourTexture;
uniform bool Trace;
int state;

int RNG()
{
    int x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 15;
    state = x;
    return x;
}

float RandomFloat01()
{
    return (RNG() & 0xFFFFFF) / 16777216.0f;
}
vec3 random_in_unit_sphere()
{
    float z = RandomFloat01() * 2.0f - 1.0f;
    float t = RandomFloat01() * 2.0f * 3.1415926f;
    float r = sqrt(max(0.0, 1.0f - z * z));
    float x = r * cos(t);
    float y = r * sin(t);
    vec3 res = vec3(x, y, z);
    res *= pow(RandomFloat01(), 1.0 / 3.0);
    return res;
}

struct Ray
{
	vec3 origin;
	vec3 dir;
};

struct Lambertian
{
	vec3 albedo;
};
struct Metal
{
	vec3 albedo;
	float fuzz;	
};
struct Dielectric
{
	float ref_idx;
};

struct Diffuse_light
{
	vec3 color;
};

struct Hit_record
{
	float t;
	vec3 p;
	vec3 normal;
	int mat;
	Metal metal;
	Lambertian lam;
	Dielectric di;
	Diffuse_light dl;
};

struct Sphere
{
	vec3 center;
	float radius;
	int mat;
	Metal metal;
	Lambertian lam;
	Dielectric di;
	Diffuse_light dl;
};

struct XY_rect
{
	float x0,x1,y0,y1,k;
	int mat;
	Metal metal;
	Lambertian lam;
	Dielectric di;
	Diffuse_light dl;
	bool flip;
};

struct XZ_rect
{
	float x0,x1,z0,z1,k;
	int mat;
	Metal metal;
	Lambertian lam;
	Dielectric di;
	Diffuse_light dl;
	bool flip;
};

struct YZ_rect
{
	float y0,y1,z0,z1,k;
	int mat;
	Metal metal;
	Lambertian lam;
	Dielectric di;
	Diffuse_light dl;
	bool flip;
};

struct Hitable_list
{
	Sphere spheres[2];
	XY_rect xyrects[100];
	XZ_rect xzrects[100];
	YZ_rect yzrects[100];
	int sphere_cnt,xyrect_cnt,xzrect_cnt,yzrect_cnt;
};

Hitable_list world;

struct Camera
{
	vec3 origin,llc,horizontal,vertical,u,v,w;
	float lens_radius;
};
vec3 random_in_unit_disk()
{
	vec3 p;
	do
	{
		p=2.0*vec3(RandomFloat01(),RandomFloat01(),0)-vec3(1,1,0);
	}while(dot(p,p)>=1.0);
	return p;
}
void Place_cam(inout Camera cam,vec3 lookfrom,vec3 lookat,vec3 vup,float vfov,float aspect,float aperture,float focus_dist)
{
	cam.lens_radius=aperture/2;
	float theta=vfov*M_PI/180;
	float half_height=tan(theta/2);
	float half_width=aspect*half_height;
	cam.origin=lookfrom;
	cam.w=normalize(lookfrom-lookat);
	cam.u=normalize(cross(vup,cam.w));
	cam.v=cross(cam.w,cam.u);
	cam.llc=cam.origin-half_width*focus_dist*cam.u-half_height*focus_dist*cam.v-focus_dist*cam.w;
	cam.horizontal=2*half_width*focus_dist*cam.u;
	cam.vertical=2*half_height*focus_dist*cam.v;
}

vec3 reflect(vec3 v,vec3 n)
{
	return v-2*dot(v,n)*n;
}

bool refract(vec3 v,vec3 n,float ni_over_nt,inout vec3 refracted)
{
	vec3 uv=normalize(v);
	float dt=dot(uv,n);
	float discriminant=1.0-ni_over_nt*ni_over_nt*(1-dt*dt);
	if(discriminant>0)
	{
		refracted=ni_over_nt*(uv-n*dt)-n*sqrt(discriminant);
		return true;
	}else
	{
		return false;
	}
}
float schlick(float cosine,float ref_idx)
{
	float r0=(1-ref_idx)/(1+ref_idx);
	r0=r0*r0;
	return r0+(1-r0)*pow((1-cosine),5);
}
bool Lam_scatter(Lambertian lam,Ray rin,Hit_record rec,inout vec3 attenuation,inout Ray scattered)
{
	vec3 target=rec.p+rec.normal+random_in_unit_sphere();
	scattered=Ray(rec.p,target-rec.p);
	attenuation=lam.albedo;
	return true;
}

vec3 Diffuse_emitted(Diffuse_light dl)
{
	return dl.color;
}


bool Metal_scatter(Metal metal,Ray rin,Hit_record rec,inout vec3 attenuation,inout Ray scattered)
{
	vec3 reflected=reflect(normalize(rin.dir),rec.normal);
	scattered=Ray(rec.p,reflected+metal.fuzz*random_in_unit_sphere());
	attenuation=metal.albedo;
	return dot(scattered.dir,rec.normal)>0;
}

bool Di_scatter(Dielectric di,Ray rin,Hit_record rec,inout vec3 attenuation,inout Ray scattered)
{
	vec3 outward_normal;
	vec3 reflected =reflect(rin.dir,rec.normal);
	float ni_over_nt;
	attenuation=vec3(1,1,1);
	vec3 refracted;
	float reflect_prob;
	float cosine;
	if(dot(rin.dir,rec.normal)>0)
	{
		outward_normal=-rec.normal;
		ni_over_nt=di.ref_idx;
		cosine=di.ref_idx*dot(rin.dir,rec.normal)/sqrt(dot(rin.dir,rin.dir));
	}else
	{
		outward_normal=rec.normal;
		ni_over_nt=1.0/di.ref_idx;
		cosine=-dot(rin.dir,rec.normal)/sqrt(dot(rin.dir,rin.dir));
	}
	if(refract(rin.dir,outward_normal,ni_over_nt,refracted))
	{
		reflect_prob=schlick(cosine,di.ref_idx);
		
	}else
	{
		scattered=Ray(rec.p,reflected);
		reflect_prob=1.0;
	}
	if(RandomFloat01()<reflect_prob)
	{
		scattered=Ray(rec.p,reflected);
	}else
	{
		scattered=Ray(rec.p,refracted);
	}
	return true;
}
Ray get_ray(Camera cam,float u,float v)
{
	vec3 rd=cam.lens_radius*random_in_unit_disk();
	vec3 offset=cam.u*rd.x+cam.v*rd.y;
	return Ray(cam.origin+offset,cam.llc+u*cam.horizontal+v*cam.vertical-cam.origin-offset);
}

vec3 point_at_param(Ray r,float t)
{
	return r.origin+t*r.dir;
}

bool hit_sphere(Sphere sp,Ray r,float tmin,float tmax,inout Hit_record rec)
{
	vec3 oc=r.origin-sp.center;
	float a=dot(r.dir,r.dir);
	float b=dot(oc,r.dir);
	float c=dot(oc,oc)-sp.radius*sp.radius;
	float discriminant=b*b-a*c;
	if(discriminant>0)
	{
		float temp=(-b-sqrt(b*b-a*c))/a;
		if(temp<tmax&&temp>tmin)
		{
			rec.t=temp;
			rec.p=point_at_param(r,rec.t);
			rec.normal=(rec.p-sp.center)/sp.radius;
			rec.mat=sp.mat;
			rec.metal=sp.metal;
			rec.lam=sp.lam;
			rec.di=sp.di;
			rec.dl=sp.dl;
			return true;
		}
		temp=(-b+sqrt(b*b-a*c))/a;
		if(temp<tmax&&temp>tmin)
		{
			rec.t=temp;
			rec.p=point_at_param(r,rec.t);
			rec.normal=(rec.p-sp.center)/sp.radius;
			rec.mat=sp.mat;
			rec.metal=sp.metal;
			rec.lam=sp.lam;
			rec.di=sp.di;
			rec.dl=sp.dl;
			return true;
		}
	}else
	{
		return false;
	}
}

bool hit_xyrect(XY_rect xyrect,Ray r,float t0,float t1,inout Hit_record rec)
{
	float t=(xyrect.k-r.origin.z)/r.dir.z;
	if(t<t0||t>t1)
		return false;
	float x=r.origin.x+t*r.dir.x;
	float y=r.origin.y+t*r.dir.y;
	if(x<xyrect.x0||x>xyrect.x1||y<xyrect.y0||y>xyrect.y1)
	{
		return false;
	}
	rec.t=t;
	rec.mat=xyrect.mat;
	rec.metal=xyrect.metal;
	rec.lam=xyrect.lam;
	rec.di=xyrect.di;
	rec.dl=xyrect.dl;
	rec.p=point_at_param(r,t);
	rec.normal=vec3(0,0,1);
		if(xyrect.flip)
		rec.normal*=-1;
	return true;
}

bool hit_xzrect(XZ_rect xzrect,Ray r,float t0,float t1,inout Hit_record rec)
{
	float t=(xzrect.k-r.origin.y)/r.dir.y;
	if(t<t0||t>t1)
		return false;
	float x=r.origin.x+t*r.dir.x;
	float z=r.origin.z+t*r.dir.z;
	if(x<xzrect.x0||x>xzrect.x1||z<xzrect.z0||z>xzrect.z1)
	{
		return false;
	}
	rec.t=t;
	rec.mat=xzrect.mat;
	rec.metal=xzrect.metal;
	rec.lam=xzrect.lam;
	rec.di=xzrect.di;
	rec.dl=xzrect.dl;
	rec.p=point_at_param(r,t);
	rec.normal=vec3(0,1,0);
		if(xzrect.flip)
		rec.normal*=-1;
	return true;
}

bool hit_yzrect(YZ_rect yzrect,Ray r,float t0,float t1,inout Hit_record rec)
{
	float t=(yzrect.k-r.origin.x)/r.dir.x;
	if(t<t0||t>t1)
		return false;
	float y=r.origin.y+t*r.dir.y;
	float z=r.origin.z+t*r.dir.z;
	if(y<yzrect.y0||y>yzrect.y1||z<yzrect.z0||z>yzrect.z1)
	{
		return false;
	}
	rec.t=t;
	rec.mat=yzrect.mat;
	rec.metal=yzrect.metal;
	rec.lam=yzrect.lam;
	rec.di=yzrect.di;
	rec.dl=yzrect.dl;
	rec.p=point_at_param(r,t);
	rec.normal=vec3(1,0,0);
	if(yzrect.flip)
		rec.normal*=-1;
	return true;
}


bool hit_list(Hitable_list hl,Ray r,float tmin,float tmax,inout Hit_record rec)
{
	Hit_record tmprec;
	bool hitanything=false;
	float closestsofar=tmax;
	for(int i=0;i<hl.sphere_cnt;i++)
	{
		if(hit_sphere(hl.spheres[i],r,tmin,closestsofar,tmprec))
		{
			hitanything=true;
			closestsofar=tmprec.t;
			rec=tmprec;
		}
	}
	for(int i=0;i<hl.xyrect_cnt;i++)
	{
		if(hit_xyrect(hl.xyrects[i],r,tmin,closestsofar,tmprec))
		{
			hitanything=true;
			closestsofar=tmprec.t;
			rec=tmprec;
		}
	}
	for(int i=0;i<hl.xzrect_cnt;i++)
	{
		if(hit_xzrect(hl.xzrects[i],r,tmin,closestsofar,tmprec))
		{
			hitanything=true;
			closestsofar=tmprec.t;
			rec=tmprec;
		}
	}
	for(int i=0;i<hl.yzrect_cnt;i++)
	{
		if(hit_yzrect(hl.yzrects[i],r,tmin,closestsofar,tmprec))
		{
			hitanything=true;
			closestsofar=tmprec.t;
			rec=tmprec;
		}
	}
	return hitanything;
}


vec3 oldcolor(Ray r)
{
	Hit_record rec;
	if(hit_list(world,r,0.0,MAXFLOAT,rec))
	{
		//vec3 target=rec.p+rec.normal+random_in_unit_sphere();
		return 0.5*vec3(rec.normal.x+1,rec.normal.y+1,rec.normal.z+1);
		//return 0.5*color(Ray(rec.p,target-rec.p));
	}else
	{
		vec3 unit_dir=normalize(r.dir);
		float t=0.5*(unit_dir.y+1);
		return (1.0-t)*vec3(1,1,1)+t*vec3(0.5,0.7,1.0);
	}
}

vec3 Raytrace1_color(Ray r)
{
	Hit_record rec;
	if(hit_list(world,r,0.001,MAXFLOAT,rec))
	{
		vec3 target=rec.p+rec.normal+random_in_unit_sphere();
		float ans=0.5;
		Ray r2=Ray(rec.p,target-rec.p);
		for(int i=0;i<50;i++)
		{
			Hit_record rec2;
			if(hit_list(world,r2,0.001,MAXFLOAT,rec2))
			{
				ans*=0.5;
				vec3 target2=rec2.p+rec2.normal+random_in_unit_sphere();
				r2=Ray(rec2.p,target2-rec2.p);
			}else
			{
				vec3 unit_dir=normalize(r2.dir);
				float t=0.5*(unit_dir.y+1);
				return ans*((1.0-t)*vec3(1,1,1)+t*vec3(0.5,0.7,1.0));
			}
		}
		return vec3(0,0,0);
		//return 0.5*color(Ray(rec.p,target-rec.p));
	}else
	{
		vec3 unit_dir=normalize(r.dir);
		float t=0.5*(unit_dir.y+1);
		return (1.0-t)*vec3(1,1,1)+t*vec3(0.5,0.7,1.0);
	}
}

vec3 color(Ray r)
{
	vec3 ans=vec3(1,1,1);
	for(int i=0;i<50;i++)
	{
		Hit_record rec;
		if(hit_list(world,r,0.001,MAXFLOAT,rec))
		{
			Ray scattered;
			vec3 attenuation;
			vec3 emitted=vec3(0,0,0);
			if(rec.mat==3)
			{
				emitted=Diffuse_emitted(rec.dl);
			}
			if((rec.mat==0&&Metal_scatter(rec.metal,r,rec,attenuation,scattered))||
			(rec.mat==1&&Lam_scatter(rec.lam,r,rec,attenuation,scattered))||
			(rec.mat==2&&Di_scatter(rec.di,r,rec,attenuation,scattered))
			)
			{
				r=scattered;
				ans*=attenuation;
			}else
			{
				return ans*emitted;
			}
		}
	}
	return vec3(0,0,0);
}

// main
void main() {
	// state = int(fragCoord.x * 1973 + fragCoord.y * 9277) | 1;
	state=int(fragCoord.x * 1973 + fragCoord.y * 9277+RandSeed) | 1;
	int ns=100;
	world.sphere_cnt=2;
	world.spheres[0]=Sphere(vec3(400,100,400),100,2,Metal(vec3(0.8,0.8,0.8),1),Lambertian(vec3(0.5,0.5,0.5)),Dielectric(1.5),Diffuse_light(vec3(8,8,8)));
	world.spheres[1]=Sphere(vec3(200,100,200),100,0,Metal(vec3(0.8,0.8,0.8),0.3),Lambertian(vec3(0.5,0.5,0.5)),Dielectric(1.5),Diffuse_light(vec3(8,8,8)));
	world.xyrect_cnt=1;
	world.xyrects[0]=XY_rect(0,555,0,555,555,1,Metal(vec3(0.8,0.8,0.8),1.0),Lambertian(vec3(0.73,0.73,0.73)),Dielectric(1.5),Diffuse_light(vec3(8,8,8)),true);
	world.yzrect_cnt=2;
	world.yzrects[0]=YZ_rect(0,555,0,555,555,1,Metal(vec3(0.8,0.8,0.8),1.0),Lambertian(vec3(0.12,0.45,0.15)),Dielectric(1.5),Diffuse_light(vec3(8,8,8)),true);
	world.yzrects[1]=YZ_rect(0,555,0,555,0,1,Metal(vec3(0.8,0.8,0.8),1.0),Lambertian(vec3(0.65,0.05,0.05)),Dielectric(1.5),Diffuse_light(vec3(8,8,8)),false);
	world.xzrect_cnt=3;
	world.xzrects[0]=XZ_rect(213,343,227,332,554,3,Metal(vec3(0.8,0.8,0.8),1.0),Lambertian(vec3(0.8,0.8,0.8)),Dielectric(1.5),Diffuse_light(vec3(15,15,15)),false);
	world.xzrects[1]=XZ_rect(0,555,0,555,555,1,Metal(vec3(0.8,0.8,0.8),1.0),Lambertian(vec3(0.73,0.73,0.73)),Dielectric(1.5),Diffuse_light(vec3(8,8,8)),true);
	world.xzrects[2]=XZ_rect(0,555,0,555,0,1,Metal(vec3(0.8,0.8,0.8),1.0),Lambertian(vec3(0.73,0.73,0.73)),Dielectric(1.5),Diffuse_light(vec3(8,8,8)),false);
	// fragColor=vec4(0,0,0,0);
	
	fragColor=texture(ourTexture,texCoord);

	Camera cam;
	// cam.llc=vec3(-2.0,-1.0,-1.0);
	// cam.horizontal=vec3(4,0,0);
	// cam.vertical=vec3(0,2,0);
	// cam.origin=vec3(0,0,0);
	vec3 lookfrom=vec3(278,278,-800);
	vec3 lookat=vec3(278,278,0);
	float dist_to_focus=10;
	float aperture=0.0;
	Place_cam(cam,lookfrom,lookat,vec3(0,1,0),40,2,aperture,dist_to_focus);
	
	for(int s=0;s<ns;s++)
	{
		float u=float(fragCoord.x+RandomFloat01())/float(iResolution.x);
		float v=float(fragCoord.y+RandomFloat01())/float(iResolution.y);

		Ray r=get_ray(cam,u,v);
		fragColor+= color(r).xyzz;
	}
	fragColor/=float(ns);
	fragColor=vec4(sqrt(fragColor.x),sqrt(fragColor.y),sqrt(fragColor.z),1);
}

// void main() {
// 	int ns=20;
// 	state = int(fragCoord.x * 1973 + fragCoord.y * 9277) | 1;
// 	world.list_size=101;
// 	world.spheres[0]=Sphere(vec3(0,-1000.0,0),1000,1,Metal(vec3(0.8,0.8,0.8),1),Lambertian(vec3(0.5,0.5,0.5)),Dielectric(1.5));
// 	int i=1;
// 	for(int a=-5;a<5;a++)
// 	{
// 		for(int b=-5;b<5;b++)
// 		{
// 			float choose_mat=RandomFloat01();
// 			vec3 center=vec3(a+0.9*RandomFloat01(),0.2,b+0.9*RandomFloat01());
// 			if(length(center-vec3(4,0.2,0))>0.9)
// 			{
// 				if(choose_mat<0.8)
// 				{
// 					world.spheres[i++]=Sphere(center,0.2,0,Metal(vec3(0.8,0.8,0.8),1),
// 					Lambertian(vec3(RandomFloat01()*RandomFloat01(),RandomFloat01()*RandomFloat01(),RandomFloat01()*RandomFloat01())),
// 					Dielectric(1.5));
// 				}else if(choose_mat<0.95)
// 				{
// 					world.spheres[i++]=Sphere(center,0.2,1,Metal(vec3(0.5*(1+RandomFloat01()),0.5*(1+RandomFloat01()),0.5*(1+RandomFloat01())),0.5*RandomFloat01()),
// 					Lambertian(vec3(0,0,0)),
// 					Dielectric(1.5));
// 				}else
// 				{
// 					world.spheres[i++]=Sphere(center,0.2,2,Metal(vec3(0,0,0),1),
// 					Lambertian(vec3(0,0,0)),
// 					Dielectric(1.5));
// 				}
// 			}
// 		}
// 	}
// 	fragColor=vec4(0,0,0,0);
// 	Camera cam;
	
// 	// cam.llc=vec3(-2.0,-1.0,-1.0);
// 	// cam.horizontal=vec3(4,0,0);
// 	// cam.vertical=vec3(0,2,0);
// 	// cam.origin=vec3(0,0,0);
// 	vec3 lookfrom=vec3(3,3,2);
// 	vec3 lookat=vec3(0,0,-1);
// 	float dist_to_focus=sqrt(dot((lookfrom-lookat),(lookfrom-lookat)));
// 	float aperture=2.0;
// 	Place_cam(cam,lookfrom,lookat,vec3(0,1,0),20,2,aperture,dist_to_focus);
	
// 	for(int s=0;s<ns;s++)
// 	{
// 		float u=float(fragCoord.x+RandomFloat01())/float(iResolution.x);
// 		float v=float(fragCoord.y+RandomFloat01())/float(iResolution.y);

// 		Ray r=get_ray(cam,u,v);
// 		vec3 p=point_at_param(r,2.0);
// 		r.origin=cam.origin;
// 		r.dir=cam.llc+u*cam.horizontal+v*cam.vertical;
// 		fragColor+= color(r).xyzz;
// 	}
// 	fragColor/=float(ns);
// 	fragColor=vec4(sqrt(fragColor.x),sqrt(fragColor.y),sqrt(fragColor.z),1);
// }