// Signal meter scene for the industrial-surfaces experiment (gui_forms).
// Not part of upstream bfft. Included after key_scene.hpp; --scene meter.
//
// The static part of a dBm meter, rendered by the engine: a recessed box under
// a weak ambient light, a scale arc, ticks and numerals glowing white, and two
// readout panels lit aqua-blue. The needles and the live numbers move and are
// drawn over this render.
//
// Design coordinates are those of a 640 × 268 drawing (x right, y down); one
// world unit is 200 of them. meter_point maps (x, y, height above the face)
// to the engine's (x, y up, z towards the bottom of the screen).

inline constexpr double meter_units=200.0;
inline Vec3 meter_point(double x,double y,double h){return {(x-320)/meter_units,h,(y-134)/meter_units};}

// A thin rectangle on the face along a design-space segment, half width w.
inline int meter_stroke(Scene& s,double x0,double y0,double x1,double y1,double w,double h,int material){
    const double dx=x1-x0,dy=y1-y0,len=std::hypot(dx,dy);if(len<1e-9)return -1;
    const double ux=dx/len,uy=dy/len;
    const Vec3 origin=meter_point(x0-ux*w+uy*w,y0-uy*w-ux*w,h);
    const Vec3 along=meter_point(x0+ux*(len+w),y0+uy*(len+w),h)-meter_point(x0-ux*w,y0-uy*w,h);
    const Vec3 across=meter_point(x0-uy*w,y0+ux*w,h)-meter_point(x0+uy*w,y0-ux*w,h);
    Vec3 n=unit(cross(along,across));if(n.y<0)n=-n;
    const int id=add_rect(s,"meter_stroke",origin,along,across,n,material,false);
    s.primitives[id].occludes=false;  // paint on the face: it glows, it casts no shadow
    return id;}

// The stroke font, with digits and the letters the meter uses.
inline const char* meter_glyph(char c){
    switch(c){
    case '0':return "1,0 0,1 0,5 1,6 3,6 4,5 4,1 3,0 1,0";
    case '1':return "1,5 2,6 2,0|1,0 3,0";
    case '2':return "0,5 1,6 3,6 4,5 4,4 0,0 4,0";
    case '3':return "0,5 1,6 3,6 4,5 4,4 3,3 4,2 4,1 3,0 1,0 0,1|1.5,3 3,3";
    case '4':return "3,0 3,6 0,2 4,2";
    case '5':return "4,6 0,6 0,3.5 3,3.5 4,2.5 4,1 3,0 1,0 0,1";
    case '6':return "4,5 3,6 1,6 0,5 0,1 1,0 3,0 4,1 4,2 3,3 0,3";
    case '7':return "0,6 4,6 1.5,0";
    case '8':return "1,3 0,4 0,5 1,6 3,6 4,5 4,4 3,3 1,3 0,2 0,1 1,0 3,0 4,1 4,2 3,3";
    case '9':return "0,1 1,0 3,0 4,1 4,5 3,6 1,6 0,5 0,4 1,3 4,3";
    case '-':return "0.6,3 3.4,3";
    case 'd':return "4,6 4,0|4,1 3,0 1,0 0,1 0,3 1,4 3,4 4,3";
    case 'B':return "0,0 0,6 3,6 4,5 4,4 3,3 0,3|3,3 4,2 4,1 3,0 0,0";
    case 'm':return "0,0 0,4|0,3 1,4 2,3 2,0|2,3 3,4 4,3 4,0";
    default:return nullptr;}}

// Text centred on (cx, cy) in design space, `size` tall, turned by `angle`
// (radians, design space), slanted for an italic face by `slant`.
inline void meter_text(Scene& s,const std::string& text,double cx,double cy,double size,double angle,double slant,double w,double h,int material){
    const double unit=size/6,width=(double(text.size())*5.5-1.5)*unit;const double ca=std::cos(angle),sa=std::sin(angle);
    double pen=-width/2;
    for(const char ch:text){const char* g=meter_glyph(ch);
        if(g){const char* cursor=g;bool previous=false;double px=0,py=0;
            while(*cursor){if(*cursor=='|'){previous=false;++cursor;continue;}
                char* end=nullptr;const double gx=std::strtod(cursor,&end);cursor=end+1;const double gy=std::strtod(cursor,&end);cursor=end;
                while(*cursor==' ')++cursor;
                // local: x along the line, y down; italic leans the top to the right
                const double lx=pen+gx*unit+slant*(gy*unit-size/2),ly=size/2-gy*unit;
                const double x=cx+lx*ca-ly*sa,y=cy+lx*sa+ly*ca;
                if(previous)meter_stroke(s,px,py,x,y,w,h,material);px=x;py=y;previous=true;}}
        pen+=5.5*unit;}}

inline Scene build_meter_scene(){Scene s;
    // materials
    const int face=add_material(s,{"meter face",MaterialKind::Diffuse,{.075,.11,.16},{},.92,.8});
    const int wall=add_material(s,{"box wall",MaterialKind::Diffuse,{.05,.06,.07},{},.9,.7});
    const int bezel=add_material(s,{"bezel",MaterialKind::Glossy,{.06,.065,.075},{},.6,.35});
    const int glow_white=add_material(s,{"scale paint",MaterialKind::Emissive,{1,1,1},{5.5,5.6,5.8},0,0});
    // the box: the face at the bottom of a recess 0.12 deep, walls, a bezel lip
    const double depth=.12;
    add_rect(s,"meter_face",meter_point(14,6,0),meter_point(626,6,0)-meter_point(14,6,0),meter_point(14,262,0)-meter_point(14,6,0),{0,1,0},face,true);
    const Vec3 up{0,depth,0};
    add_rect(s,"wall_top",meter_point(14,6,0),meter_point(626,6,0)-meter_point(14,6,0),up,{0,0,1},wall,true);
    add_rect(s,"wall_bottom",meter_point(14,262,0),meter_point(626,262,0)-meter_point(14,262,0),up,{0,0,-1},wall,true);
    add_rect(s,"wall_left",meter_point(14,6,0),meter_point(14,262,0)-meter_point(14,6,0),up,{1,0,0},wall,true);
    add_rect(s,"wall_right",meter_point(626,6,0),meter_point(626,262,0)-meter_point(626,6,0),up,{-1,0,0},wall,true);
    add_rect(s,"bezel_top",meter_point(0,0,depth),meter_point(640,0,depth)-meter_point(0,0,depth),meter_point(0,6,depth)-meter_point(0,0,depth),{0,1,0},bezel,false);
    add_rect(s,"bezel_bottom",meter_point(0,262,depth),meter_point(640,262,depth)-meter_point(0,262,depth),meter_point(0,268,depth)-meter_point(0,262,depth),{0,1,0},bezel,false);
    add_rect(s,"bezel_left",meter_point(0,6,depth),meter_point(14,6,depth)-meter_point(0,6,depth),meter_point(0,262,depth)-meter_point(0,6,depth),{0,1,0},bezel,false);
    add_rect(s,"bezel_right",meter_point(626,6,depth),meter_point(640,6,depth)-meter_point(626,6,depth),meter_point(626,262,depth)-meter_point(626,6,depth),{0,1,0},bezel,false);

    // the scale: an arc about a pivot far below the face, ticks, numerals
    const double px=320,py=760,R=660,sweep=23.2*pi/180;const double paint_h=.001;
    auto angle_of=[&](double db){return -pi/2+((db+140)/140-.5)*2*sweep;};
    const int arc_pieces=48;
    for(int i=0;i<arc_pieces;++i){const double a0=angle_of(-140)+(angle_of(0)-angle_of(-140))*i/arc_pieces,a1=angle_of(-140)+(angle_of(0)-angle_of(-140))*(i+1)/arc_pieces;
        meter_stroke(s,px+R*std::cos(a0),py+R*std::sin(a0),px+R*std::cos(a1),py+R*std::sin(a1),2.0,paint_h,glow_white);}
    for(int db=-140;db<=0;db+=10){const double a=angle_of(db);const bool major=db%20==0;
        meter_stroke(s,px+(R+2)*std::cos(a),py+(R+2)*std::sin(a),px+(R+(major?24:15))*std::cos(a),py+(R+(major?24:15))*std::sin(a),major?1.5:.8,paint_h,glow_white);
        if(major)meter_text(s,std::to_string(db),px+(R+42)*std::cos(a),py+(R+42)*std::sin(a),16,a+pi/2,.18,.9,paint_h,glow_white);}
    meter_text(s,"dBm",58,32,20,0,0,1.1,paint_h,glow_white);

    // Readout panels: backlit LED panels standing above the face (the needles
    // pass beneath them). Each is a thin slab of white translucent plastic, the
    // engine's participating medium, with three aqua LEDs inside behind its
    // front and a grid of hidden cores sampling their light; nothing spills
    // past the panel.
    const std::array<std::array<double,4>,2> readouts{{{176,172,128,50},{332,172,128,50}}};
    Material diffuser{"panel diffuser",MaterialKind::Dielectric,{.99,.99,.99},{},.92,.02,0,1.49,{1.49,1.49,1.49},{45,45,45},false};
    const int plastic=add_material(s,diffuser);
    const int core_white=add_material(s,{"panel core",MaterialKind::Diffuse,{.92,.92,.9},{},.95,.9});
    const int led=add_material(s,{"aqua LED",MaterialKind::Emissive,{1,1,1},{100,190,220},0,0});
    // deep enough that the cores (radius .012) sit clear above the LEDs, as a
    // backlit panel's light cavity does
    const double back_h=.012,front_h=.075;
    for(const auto& r:readouts){
        const std::array<Vec3,8> v{{meter_point(r[0],r[1],back_h),meter_point(r[0]+r[2],r[1],back_h),meter_point(r[0]+r[2],r[1]+r[3],back_h),meter_point(r[0],r[1]+r[3],back_h),
            meter_point(r[0],r[1],front_h),meter_point(r[0]+r[2],r[1],front_h),meter_point(r[0]+r[2],r[1]+r[3],front_h),meter_point(r[0],r[1]+r[3],front_h)}};
        const Vec3 inside=meter_point(r[0]+r[2]/2,r[1]+r[3]/2,(back_h+front_h)/2);
        const std::array<std::array<int,4>,6> faces{{{0,1,2,3},{4,5,6,7},{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7}}};
        Scene::ParticipatingVolume volume;volume.first=static_cast<int>(s.primitives.size());
        for(const auto& f:faces){add_outward_triangle(s,"panel",v[f[0]],v[f[1]],v[f[2]],inside,plastic,false);
            add_outward_triangle(s,"panel",v[f[0]],v[f[2]],v[f[3]],inside,plastic,false);}
        volume.last=static_cast<int>(s.primitives.size())-1;
        for(int id=volume.first;id<=volume.last;++id)s.primitives[id].occludes=false;  // it transmits
        for(int j=0;j<3;++j)for(int i=0;i<6;++i){
            const int core=add_sphere(s,"panel_core",meter_point(r[0]+r[2]*(i+.5)/6,r[1]+r[3]*(j+.5)/3,back_h+.04),.012,core_white,true);
            s.primitives[core].intersectable=false;volume.cores.push_back(core);}
        if(s.jelly_volume<0){s.jelly_volume=volume.first;s.jelly_core=volume.cores[0];}
        s.participating_volumes.push_back(volume);
        // three LEDs along the panel, just above its back, facing its front
        for(int l=0;l<3;++l){const double cx=r[0]+r[2]*(l+.5)/3,cy=r[1]+r[3]/2;
            // wound so that its edges' cross product is its normal (+y, towards the front)
            const int id=add_rect(s,"led",meter_point(cx-5,cy+5,back_h+.004),meter_point(cx+5,cy+5,0)-meter_point(cx-5,cy+5,0),
                meter_point(cx-5,cy-5,0)-meter_point(cx-5,cy+5,0),{0,1,0},led,false);
            s.primitives[id].intersectable=false;s.area_lights.push_back({id,s.materials[led].emission});}}

    // Light. The weak ambient: a broad hidden panel well above the box.
    const int ambient=add_material(s,{"ambient",MaterialKind::Emissive,{1,1,1},{.9,.93,1.0},0,0});
    // above and towards the top of the screen, so the top wall shades the face
    {const int r=add_rect(s,"ambient_panel",{-2.2,2.6,-3.4},{4.4,0,0},{0,0,2.4},{0,-1,0},ambient,false);
        s.primitives[r].intersectable=false;s.area_lights.push_back({r,s.materials[ambient].emission});}
    // The scale's glow on the face: hidden strips just above the arc, facing it.
    // High enough (0.12) and wide enough (40 design units) that their light
    // spreads into an even pool along the scale rather than a row of spots.
    const int arc_light=add_material(s,{"scale glow",MaterialKind::Emissive,{1,1,1},{.42,.43,.46},0,0});
    for(int i=0;i<12;++i){const double a0=angle_of(-140)+(angle_of(0)-angle_of(-140))*i/12,a1=angle_of(-140)+(angle_of(0)-angle_of(-140))*(i+1)/12;
        const double x0=px+R*std::cos(a0),y0=py+R*std::sin(a0),x1=px+R*std::cos(a1),y1=py+R*std::sin(a1);
        const double len=std::hypot(x1-x0,y1-y0),nx=-(y1-y0)/len*20,ny=(x1-x0)/len*20;
        const Vec3 o=meter_point(x0-nx,y0-ny,.11),along=meter_point(x1,y1,.11)-meter_point(x0,y0,.11),across=meter_point(nx,ny,0)-meter_point(-nx,-ny,0);
        const int r=add_rect(s,"scale_glow",o,along,across,{0,-1,0},arc_light,false);
        s.primitives[r].intersectable=false;s.area_lights.push_back({r,s.materials[arc_light].emission});}
    build_scene_bvh(s);return s;}
