// tracer-bakes playground scenes, written in the engine's own geometry (not
// part of upstream bfft). Included by regime_scene_native.cpp after its scene
// builders; selected with --scene tracer --tracer-scene <name> --tracer-pass
// <fixed|lamp>.
//
// The playground's four control-sized scenes, given to the engine as it
// expects them: rectangles (tiled, because the engine lights each transport
// patch with one value), spheres and rectangular area lights, whose source
// windows the engine integrates exactly. Rounded bevels and domes are not
// modelled: boxes and spheres stand for them. Dimensions follow
// experiments/tracer-bakes/scenes.hpp.
//
// Coordinates: the chart is the unit square, x right, y up the screen, h out
// of the panel; tracer_point maps chart (x, y, h) to the engine as
// (x − ½, h, ½ − y), as key_scene.hpp does, and the camera looks straight down.
//
// Passes: "fixed" lights the scene with the one fixed light (a distant
// rectangle up and to the left, angular radius about 0.06, unit irradiance at
// normal incidence) and a dim sky; the lamp's lens is a cold diffuser.
// "lamp" lights it with the lamp alone at unit radiance: a visible emissive
// lens and a hidden box of five rectangular area lights around the lamp.

struct TracerOptions {
    std::string scene="keycap";  // keycap, lamp, contact, well
    std::string pass="fixed";    // fixed, lamp
};
inline TracerOptions tracer_options;

inline Vec3 tracer_point(double x,double y,double h){return {x-.5,h,.5-y};}

// An axis-aligned rectangle of the chart, tiled nx × ny, at height h, facing up.
inline void tracer_floor(Scene& s,const std::string& name,double x0,double y0,double x1,double y1,double h,int material,int nx,int ny){
    for(int j=0;j<ny;++j)for(int i=0;i<nx;++i){
        const double ax=x0+(x1-x0)*i/nx,bx=x0+(x1-x0)*(i+1)/nx,ay=y0+(y1-y0)*j/ny,by=y0+(y1-y0)*(j+1)/ny;
        add_rect(s,name,tracer_point(ax,by,h),tracer_point(bx,by,h)-tracer_point(ax,by,h),
                 tracer_point(ax,ay,h)-tracer_point(ax,by,h),{0,1,0},material,true);}}

// A vertical wall along the chart segment (x0, y0)–(x1, y1) from h0 to h1,
// facing (nx, ny) in the chart, tiled along its length and height.
inline void tracer_wall(Scene& s,const std::string& name,double x0,double y0,double x1,double y1,double h0,double h1,
                        double nx,double ny,int material,int along,int up){
    for(int j=0;j<up;++j)for(int i=0;i<along;++i){
        const double t0=double(i)/along,t1=double(i+1)/along,g0=h0+(h1-h0)*j/up,g1=h0+(h1-h0)*(j+1)/up;
        const Vec3 a=tracer_point(x0+(x1-x0)*t0,y0+(y1-y0)*t0,g0);
        const Vec3 b=tracer_point(x0+(x1-x0)*t1,y0+(y1-y0)*t1,g0);
        const Vec3 c=tracer_point(x0+(x1-x0)*t0,y0+(y1-y0)*t0,g1);
        add_rect(s,name,a,b-a,c-a,tracer_point(nx,ny,0)-tracer_point(0,0,0),material,true);}}

// A box standing on h0 with its top at h1: a top and four outward walls.
inline void tracer_box(Scene& s,const std::string& name,double x0,double y0,double x1,double y1,double h0,double h1,int material,int tiles){
    tracer_floor(s,name+"_top",x0,y0,x1,y1,h1,material,tiles,std::max(1,int(tiles*(y1-y0)/(x1-x0))));
    const int up=std::max(1,tiles/4);
    tracer_wall(s,name+"_side",x0,y1,x1,y1,h0,h1,0,1,material,tiles,up);
    tracer_wall(s,name+"_side",x1,y0,x0,y0,h0,h1,0,-1,material,tiles,up);
    tracer_wall(s,name+"_side",x0,y0,x0,y1,h0,h1,-1,0,material,tiles,up);
    tracer_wall(s,name+"_side",x1,y1,x1,y0,h0,h1,1,0,material,tiles,up);}

// The plate at h = 0 around a square hole of half size `hole` (0: no hole).
inline void tracer_plate(Scene& s,double hole,int material,int tiles){
    if(hole<=0){tracer_floor(s,"plate",0,0,1,1,0,material,tiles,tiles);return;}
    const double a=.5-hole,b=.5+hole;const int side=std::max(1,tiles/4);
    tracer_floor(s,"plate",0,b,1,1,0,material,tiles,side);
    tracer_floor(s,"plate",0,0,1,a,0,material,tiles,side);
    tracer_floor(s,"plate",0,a,a,b,0,material,side,tiles/2);
    tracer_floor(s,"plate",b,a,1,b,0,material,side,tiles/2);}

// A square well of half size `hole` down to `depth` below the plate: four
// inward walls and a floor.
inline void tracer_well(Scene& s,double hole,double depth,int wall,int floor,int tiles){
    const double a=.5-hole,b=.5+hole;const int up=std::max(2,tiles/2);
    tracer_wall(s,"well_wall",a,b,b,b,-depth,0,0,-1,wall,tiles,up);
    tracer_wall(s,"well_wall",b,a,a,a,-depth,0,0,1,wall,tiles,up);
    tracer_wall(s,"well_wall",a,a,a,b,-depth,0,1,0,wall,tiles,up);
    tracer_wall(s,"well_wall",b,b,b,a,-depth,0,-1,0,wall,tiles,up);
    tracer_floor(s,"well_floor",a,a,b,b,-depth,floor,tiles,tiles);}

// A rectangle area light facing `facing` (engine coordinates), centred at c,
// of side `side`; seen by nothing, it only lights.
inline void tracer_area_light(Scene& s,const std::string& name,Vec3 c,Vec3 facing,double side,double radiance){
    const Vec3 n=unit(facing);const Vec3 helper=std::abs(n.y)<.9?Vec3{0,1,0}:Vec3{1,0,0};
    const Vec3 u=unit(cross(helper,n))*side,v=unit(cross(n,u))*side;
    const int material=add_material(s,{name,MaterialKind::Emissive,{1,1,1},{radiance,radiance,radiance},0,0});
    const int r=add_rect(s,name,c-u*.5-v*.5,u,v,n,material,false);
    s.primitives[r].intersectable=false;s.area_lights.push_back({r,s.materials[material].emission});}

// The lamp: in the lamp pass, a visible emissive lens (it casts no shadow) and
// a hidden box of five area lights of unit radiance round its centre; in the
// fixed pass, a cold white lens.
inline void tracer_lamp(Scene& s,const TracerOptions& o,Vec3 lens_centre,double lens_radius,Vec3 lamp_centre,double lamp_half){
    const bool on=o.pass=="lamp";
    const int lens=on?add_material(s,{"lens",MaterialKind::Emissive,{1,1,1},{1,1,1},0,0})
                     :add_material(s,{"cold lens",MaterialKind::Diffuse,{.92,.92,.90},{},.95,.9});
    const int b=add_sphere(s,"lens",lens_centre,lens_radius,lens,false);s.primitives[b].occludes=false;
    if(!on)return;
    tracer_area_light(s,"lamp",lamp_centre+Vec3{0,lamp_half,0},{0,1,0},2*lamp_half,1.0);
    tracer_area_light(s,"lamp",lamp_centre+Vec3{lamp_half,0,0},{1,0,0},2*lamp_half,1.0);
    tracer_area_light(s,"lamp",lamp_centre+Vec3{-lamp_half,0,0},{-1,0,0},2*lamp_half,1.0);
    tracer_area_light(s,"lamp",lamp_centre+Vec3{0,0,lamp_half},{0,0,1},2*lamp_half,1.0);
    tracer_area_light(s,"lamp",lamp_centre+Vec3{0,0,-lamp_half},{0,0,-1},2*lamp_half,1.0);}

// The fixed light from the upper left and a dim sky overhead.
inline void tracer_fixed_light(Scene& s,const TracerOptions& o){
    if(o.pass!="fixed")return;
    // chart direction (−0.45, 0.55, 0.70), normalised, in engine coordinates
    const Vec3 towards=unit(Vec3{-.45,.70,-.55});const double distance=6,side=2*distance*.06;
    // unit irradiance at normal incidence: radiance × solid angle (side² / distance²) = 1
    tracer_area_light(s,"fixed_light",towards*distance,towards*-1,side,distance*distance/(side*side));
    tracer_area_light(s,"sky",{0,4,0},{0,-1,0},8,.22);}

inline Scene build_tracer_scene(const TracerOptions& o){Scene s;
    // the playground's materials (experiments/tracer-bakes/scenes.hpp)
    const int plate=add_material(s,{"plate",MaterialKind::Diffuse,{.40,.43,.38},{},.95,.9});
    const int floor=add_material(s,{"well floor",MaterialKind::Diffuse,{.16,.16,.15},{},.95,.9});
    const int cap=add_material(s,{"ivory cap",MaterialKind::Diffuse,{.80,.77,.68},{},.95,.9});
    const int bezel=add_material(s,{"bezel",MaterialKind::Diffuse,{.52,.53,.55},{},.95,.9});
    const int wall=add_material(s,{"white enamel",MaterialKind::Diffuse,{.86,.85,.82},{},.95,.9});
    if(o.scene=="keycap"){
        tracer_plate(s,.36,plate,8);tracer_well(s,.36,.12,floor,floor,6);
        tracer_box(s,"keycap",.21,.21,.79,.79,-.12,.06,cap,6);
        tracer_lamp(s,o,tracer_point(.68,.68,.06),.035,tracer_point(.68,.68,.075),.022);}
    else if(o.scene=="lamp"){
        tracer_plate(s,0,plate,8);
        // a square bezel frame (outer half .32, inner half .20, height .10) round a seat at .015
        tracer_box(s,"bezel",.18,.70,.82,.82,0,.10,bezel,6);tracer_box(s,"bezel",.18,.18,.82,.30,0,.10,bezel,6);
        tracer_box(s,"bezel",.18,.30,.30,.70,0,.10,bezel,3);tracer_box(s,"bezel",.70,.30,.82,.70,0,.10,bezel,3);
        tracer_floor(s,"seat",.30,.30,.70,.70,.015,bezel,4,4);
        tracer_lamp(s,o,tracer_point(.5,.5,-.08),.17,tracer_point(.5,.5,.05),.07);}
    else if(o.scene=="contact"){
        tracer_plate(s,0,plate,10);
        tracer_box(s,"bar",.15,.42,.85,.58,0,.05,cap,8);
        tracer_box(s,"strip",.15,.17,.85,.23,0,.012,bezel,8);
        tracer_lamp(s,o,tracer_point(.78,.5,.05),.025,tracer_point(.78,.5,.056),.014);}
    else if(o.scene=="well"){
        tracer_plate(s,.30,plate,8);tracer_well(s,.30,.30,wall,wall,6);
        tracer_lamp(s,o,tracer_point(.5,.5,-.30),.06,tracer_point(.5,.5,-.265),.035);}
    else throw std::runtime_error("unknown tracer scene: "+o.scene);
    tracer_fixed_light(s,o);
    build_scene_bvh(s);return s;}
