// Illuminated key scene for the industrial-surfaces experiment (gui_forms).
// Not part of upstream bfft. Included by regime_scene_native.cpp after its
// scene builders; selected with --scene key.
//
// A key is rendered on its own: no room and no room light. The only light is
// the key's lamps, so each render is one lamp's light at unit power (or none,
// or a coverage pass), and a frame is composed later as Σ power × colour × lamp.
//
// Coordinates: the engine's y is up out of the panel. The key's own drawing
// coordinates are (x right, y up the screen, h above the panel); key_point maps
// them to the engine as (x, h, −y), and the camera looks straight down with
// the screen's up along −z.
//
// The lens is a convex polyhedron of triangles. A clear lens is the engine's
// ordinary dielectric. An opaque lens is the engine's participating medium
// (the "jelly"), generalised from one sphere to this polyhedron: its chord
// exits by the faces' supporting planes, and its source function is the
// retained radiance of a hidden diffuse core lit by the lamps, so a pigmented
// diffuser's colour is carried by transported light.

struct KeyOptions {
    std::string style="cancel";  // cancel: opaque white; ok: clear, black back
    int lamp=-1;                 // which lamp emits: -1 both, 0, 1, -2 none
    bool coverage=false;         // render the key's coverage instead of light
    double press=0;              // how far the key is pushed down
    double sidewall=0.12;        // the lens's height above the panel
    bool fill=false;             // a faint fill light from above (the "room" pass), lamps off
    std::string mesh;            // the lens from a stx_conv-reconstructed mesh (cpp/key_mesh.cpp)
    double relief=0;             // the top's moulded orange-peel waves: largest slope, radians
    double grain=0;              // the top's finer stipple: largest slope, radians
};
inline KeyOptions key_options;

inline Vec3 key_point(double x,double y,double h){return {x,h,-y};}

// A triangle wound so that its normal points away from `inside`.
inline int add_outward_triangle(Scene& s,const std::string& name,Vec3 a,Vec3 b,Vec3 c,Vec3 inside,int material,bool transport){
    const Vec3 n=cross(b-a,c-a);const Vec3 centre=(a+b+c)/3;
    if(dot(n,centre-inside)<0)std::swap(b,c);
    return add_triangle(s,name,a,b,c,material,transport);}

// A rounded rectangle at height h: half sizes hx, hy, corner radius r, each
// corner sampled at key_corner_points points.
inline constexpr int key_corner_points=7;
inline constexpr int key_ring_points=4*key_corner_points;
inline std::vector<Vec3> key_ring(double hx,double hy,double r,double h){std::vector<Vec3> ring;ring.reserve(key_ring_points);
    const std::array<double,4> cx{{hx-r,-hx+r,-hx+r,hx-r}},cy{{hy-r,hy-r,-hy+r,-hy+r}};
    for(int corner=0;corner<4;++corner)for(int i=0;i<key_corner_points;++i){
        const double a=(corner+double(i)/(key_corner_points-1))*pi/2;
        ring.push_back(key_point(cx[corner]+r*std::cos(a),cy[corner]+r*std::sin(a),h));}
    return ring;}

// The angular stroke font of the experiment's lamps.cpp, as polylines on a
// 4 × 6 grid, '|' between strokes.
struct KeyGlyph{char letter;const char* strokes;};
inline constexpr std::array<KeyGlyph,9> key_glyphs{{
    {'A',"0,0 0,4.5 1.5,6 2.5,6 4,4.5 4,0|0,3 4,3"},
    {'C',"4,5 3,6 1,6 0,5 0,1 1,0 3,0 4,1"},
    {'E',"4,6 0,6 0,0 4,0|0,3 3,3"},
    {'K',"0,0 0,6|4,6 0,2|1.3,3.3 4,0"},
    {'L',"0,6 0,0 4,0"},
    {'N',"0,0 0,6 4,0 4,6"},
    {'O',"1,0 0,1 0,5 1,6 3,6 4,5 4,1 3,0 1,0"},
    {'R',"0,0 0,6 3,6 4,5 4,4 3,3 0,3|2,3 4,0"},
    {'S',"4,5 3,6 1,6 0,5 0,4 1,3 3,3 4,2 4,1 3,0 1,0 0,1"}}};

// Legend ink: each polyline of a glyph is one ribbon lying on the face, its
// joints mitred (so corners are sharp, with nothing standing out past them)
// and its two ends square, extended by the half width. A closed outline (the
// O) has no ends: its last point is its first, and every joint is mitred.
inline void add_ink_ribbon(Scene& s,std::vector<std::array<double,2>> points,double face_h,double half_width,int material){
    const bool closed=points.size()>3&&std::hypot(points.front()[0]-points.back()[0],points.front()[1]-points.back()[1])<1e-9;
    if(closed)points.pop_back();
    const std::size_t n=points.size();if(n<2)return;
    std::vector<std::array<double,2>> left(n),right(n);
    for(std::size_t k=0;k<n;++k){
        // directions of the segments before and after this point
        double ax=0,ay=0,bx=0,by=0;
        const bool has_before=k>0||closed,has_after=k+1<n||closed;
        const std::size_t before=(k+n-1)%n,after=(k+1)%n;
        if(has_before){ax=points[k][0]-points[before][0];ay=points[k][1]-points[before][1];const double l=std::hypot(ax,ay);ax/=l;ay/=l;}
        if(has_after){bx=points[after][0]-points[k][0];by=points[after][1]-points[k][1];const double l=std::hypot(bx,by);bx/=l;by/=l;}
        if(!has_before){ax=bx;ay=by;}
        if(!has_after){bx=ax;by=ay;}
        // the miter: the sum of the two segment normals, lengthened so the
        // ribbon keeps its width along both segments (limited at sharp turns)
        double mx=-(ay+by),my=ax+bx;const double ml=std::hypot(mx,my);
        if(ml<1e-9){mx=-ay;my=ax;}else{mx/=ml;my/=ml;}
        const double cosine=mx*(-ay)+my*ax;const double reach=half_width/std::max(cosine,.35);
        double cx=points[k][0],cy=points[k][1];
        if(!has_before){cx-=ax*half_width;cy-=ay*half_width;}
        if(!has_after){cx+=bx*half_width;cy+=by*half_width;}
        left[k]={cx+mx*reach,cy+my*reach};right[k]={cx-mx*reach,cy-my*reach};}
    const Vec3 below=key_point(points[0][0],points[0][1],face_h-1);
    const std::size_t segments=closed?n:n-1;
    for(std::size_t k=0;k<segments;++k){const std::size_t m=(k+1)%n;
        const Vec3 l0=key_point(left[k][0],left[k][1],face_h),r0=key_point(right[k][0],right[k][1],face_h);
        const Vec3 l1=key_point(left[m][0],left[m][1],face_h),r1=key_point(right[m][0],right[m][1],face_h);
        // ink is a film on the face: seen, but it casts no shadow into the plastic
        const int first=add_outward_triangle(s,"legend_ink",l0,r0,r1,below,material,false);
        const int second=add_outward_triangle(s,"legend_ink",l0,r1,l1,below,material,false);
        s.primitives[first].occludes=false;s.primitives[second].occludes=false;}}

inline void add_legend(Scene& s,const std::string& text,double baseline,double height,double face_h,double half_width,int material){
    const double grid=height/6;double pen=-0.5*((double(text.size())*5.5-1.5)*grid);
    for(const char letter:text){const KeyGlyph* glyph=nullptr;for(const KeyGlyph& g:key_glyphs)if(g.letter==letter)glyph=&g;
        if(glyph){const char* cursor=glyph->strokes;std::vector<std::array<double,2>> line;
            while(true){
                if(*cursor=='|'||*cursor==0){
                    add_ink_ribbon(s,line,face_h,half_width,material);line.clear();
                    if(*cursor==0)break;++cursor;continue;}
                char* end=nullptr;const double gx=std::strtod(cursor,&end);cursor=end+1;const double gy=std::strtod(cursor,&end);cursor=end;
                while(*cursor==' ')++cursor;line.push_back({pen+gx*grid,baseline+gy*grid});}}
        pen+=5.5*grid;}}

// A jewel indicator: a faceted glass crown (flat facets, alternate rings
// half a facet out of phase, as a brilliant's), a mirror back, and a small
// white-hot coil inside. Only the crown is framed; in use it shows through a
// dot in the panel.
inline Scene build_jewel_scene(const KeyOptions& o){Scene s;
    Material glass{"jewel glass",MaterialKind::Dielectric,{.99,.99,1},{},0,.005,0,1.52,{1.52,1.52,1.52},{.02,.02,.02},false};
    const int jewel=o.coverage?add_material(s,{"coverage",MaterialKind::Emissive,{1,1,1},{1,1,1},0,0}):add_material(s,glass);
    const int mirror=o.coverage?jewel:add_material(s,{"jewel back",MaterialKind::Mirror,{.92,.93,.95},{},0,0,1});
    // rings: radius, height, phase (in facets); 16 facets round
    const int n=16;
    const std::array<std::array<double,3>,5> rings{{{.10,.22,0},{.19,.184,.5},{.26,.114,0},{.30,.03,.5},{.30,-.08,.5}}};
    auto ring_point=[&](int r,int i){const double a=2*pi*(i+rings[r][2])/n;return key_point(rings[r][0]*std::cos(a),rings[r][0]*std::sin(a),rings[r][1]);};
    const Vec3 inside=key_point(0,0,.07);
    for(int i=1;i+1<n;++i)add_outward_triangle(s,"table",ring_point(0,0),ring_point(0,i),ring_point(0,i+1),inside,jewel,false);
    for(int r=0;r+1<4;++r)for(int i=0;i<n;++i){const int j=(i+1)%n;
        // the next ring is half a facet ahead or behind: zigzag triangles make the facets
        add_outward_triangle(s,"crown",ring_point(r,i),ring_point(r,j),ring_point(r+1,rings[r+1][2]>rings[r][2]?i:j),inside,jewel,false);
        add_outward_triangle(s,"crown",ring_point(r+1,i),ring_point(r+1,j),ring_point(r,rings[r+1][2]>rings[r][2]?j:i),inside,jewel,false);}
    for(int i=0;i<n;++i){const int j=(i+1)%n;  // girdle
        add_outward_triangle(s,"girdle",ring_point(3,i),ring_point(3,j),ring_point(4,j),inside,jewel,false);
        add_outward_triangle(s,"girdle",ring_point(3,i),ring_point(4,j),ring_point(4,i),inside,jewel,false);}
    for(int i=1;i+1<n;++i)add_outward_triangle(s,"back",ring_point(4,0),ring_point(4,i),ring_point(4,i+1),inside,mirror,false);
    if(o.coverage){build_scene_bvh(s);return s;}
    for(std::size_t id=0;id<s.primitives.size();++id)s.primitives[id].occludes=false;
    // the lamp: a small frosted bulb inside, low in the jewel; every facet
    // picks up a bright patch of it
    const int bulb=add_material(s,{"bulb",MaterialKind::Emissive,{1,1,1},{6,6,6},0,0});
    const int cold=add_material(s,{"cold bulb",MaterialKind::Diffuse,{.25,.25,.24},{},.9,.5});
    const int b=add_sphere(s,"bulb",key_point(0,0,-.01),.06,o.lamp==-2?cold:bulb,false);
    s.primitives[b].occludes=false;
    if(o.fill){
        const int fill=add_material(s,{"fill",MaterialKind::Emissive,{1,1,1},{.35,.35,.35},0,0});
        const int r=add_rect(s,"fill_panel",{-1.2,1.6,-1.6},{2.4,0,0},{0,0,2.0},{0,-1,0},fill,false);
        s.primitives[r].intersectable=false;s.area_lights.push_back({r,s.materials[fill].emission});}
    build_scene_bvh(s);return s;}

inline Scene build_key_scene(const KeyOptions& o){if(o.style=="jewel")return build_jewel_scene(o);Scene s;
    const bool opaque=o.style=="cancel";
    const double aspect=.62,half_base=.34,half_top=.315-.12*std::max(0.0,o.sidewall-.12);
    const double bottom=-.07-o.press,top=o.sidewall-o.press,chamfer=.022;
    const Vec3 inside=key_point(0,0,(top+bottom)*.5);

    // materials
    // Black ink reflects about 3 %: it is drawn as a surface of zero emission,
    // which is exactly black and needs no irradiance sampling over its strokes.
    const int ink=add_material(s,{"legend ink",MaterialKind::Emissive,{0,0,0},{0,0,0},0,0});
    const int cover=add_material(s,{"coverage",MaterialKind::Emissive,{1,1,1},{1,1,1},0,0});
    // Opaque: a participating diffuser. diffuse is its scattering albedo and
    // absorption its extinction density, as for the engine's jelly.
    Material opal{"white translucent",MaterialKind::Dielectric,{.99,.99,.99},{},.92,.02,0,1.49,{1.49,1.49,1.49},{14,14,14},false};
    // Clear: untinted acrylic. The two styles are clear acrylic and white
    // plastic; a key's colour is its lamp's, never its material's.
    // Its faint haze (extinction 1.2 per unit, so about 70 % passes straight
    // through the key's depth) is the same participating medium as the white
    // plastic's, at a twelfth of its density: the coil's light diffuses out
    // through the acrylic while the key stays see-through.
    Material clear{"clear acrylic",MaterialKind::Dielectric,{.99,.99,1},{},.9,.01,0,1.49,{1.49,1.49,1.49},{1.2,1.2,1.2},false};
    const int lens=o.coverage?cover:add_material(s,opaque?opal:clear);
    // A clear key's back is a matte black void: unlit, the key is dark glass
    // over black and the cold coil, as black, vanishes into it; lit, the coil
    // glows in the acrylic. (The engine's Glossy law always returns at least
    // 4% of the room, which grew a grey back with the coil silhouetted on it.)
    const int frosted=o.coverage?cover:add_material(s,{"black back",MaterialKind::Diffuse,{.01,.01,.01},{},.95,.9});
    // the sidewalls are the key's own material (black paint read as an outline)
    const int sides=o.coverage?cover:lens;

    // The lens, the shape of the experiment's captions: tapered sides with
    // rounded corners, a rounded (quarter-round) chamfer into a flat top, and
    // a flat back. Convex, so the engine's supporting-plane exits hold.
    const double corner_radius=.045,round_steps=5;
    std::vector<std::vector<Vec3>> rings;
    rings.push_back(key_ring(half_base,half_base*aspect,corner_radius,bottom));               // base
    rings.push_back(key_ring(half_top,half_top*aspect,corner_radius,top-chamfer));            // shoulder
    for(int i=1;i<=round_steps;++i){const double a=i*(pi/2)/round_steps;                        // the round
        const double inset=chamfer*(1-std::cos(a)),rise=chamfer*std::sin(a);
        rings.push_back(key_ring(half_top-inset,half_top*aspect-inset,std::max(.004,corner_radius-inset),top-chamfer+rise));}
    const int first=static_cast<int>(s.primitives.size());
    if(!o.mesh.empty()){
        // int32 count, then per triangle 9 float64 (key space) and int32 tag:
        // 0 lens face, 1 sidewall, 2 back
        std::ifstream in(o.mesh,std::ios::binary);std::int32_t count=0;
        if(!in.read(reinterpret_cast<char*>(&count),sizeof(count)))throw std::runtime_error("cannot read key mesh");
        for(std::int32_t t=0;t<count;++t){std::array<double,9> p{};std::int32_t tag=0;
            in.read(reinterpret_cast<char*>(p.data()),sizeof(p));in.read(reinterpret_cast<char*>(&tag),sizeof(tag));
            if(!in)throw std::runtime_error("truncated key mesh");
            const Vec3 a=key_point(p[0],p[1],p[2]-o.press),b=key_point(p[3],p[4],p[5]-o.press),c=key_point(p[6],p[7],p[8]-o.press);
            if(norm(cross(b-a,c-a))<1e-14)continue;  // the closed centre ring leaves degenerate triangles
            // A clear key's frosted back is laid as a grid of tiles below (the
            // engine lights each surface patch with one value; the mesh's back
            // fan of long thin triangles would light as blobs).
            if(tag==2&&!opaque&&!o.coverage)continue;
            const int material=tag==1?sides:(tag==2?(opaque?lens:frosted):lens);
            add_outward_triangle(s,tag==1?"lens_side":(tag==2?"lens_back":"lens_face"),a,b,c,inside,material,false);}
        if(!opaque&&!o.coverage){
            const double hx=half_base-.012,hy=half_base*aspect-.012;const int nx=16,ny=8;
            for(int j=0;j<ny;++j)for(int i=0;i<nx;++i){
                const double x0=-hx+2*hx*i/nx,y0=-hy+2*hy*j/ny,dx=2*hx/nx,dy=2*hy/ny;
                // keep inside the rounded corners
                const double cx=std::abs(x0+dx/2),cy=std::abs(y0+dy/2),r=.045;
                if(cx>hx-r&&cy>hy-r&&std::hypot(cx-(hx-r),cy-(hy-r))>r)continue;
                // wound so that its normal faces up into the lens
                add_rect(s,"frosted_back",key_point(x0,y0+dy,bottom+.0015),key_point(dx,0,0),key_point(0,-dy,0),{0,1,0},frosted,true);}}}
    else{
    const std::vector<Vec3>& face=rings.back();
    for(int i=1;i+1<key_ring_points;++i)add_outward_triangle(s,"lens_top",face[0],face[i],face[i+1],inside,lens,false);
    for(std::size_t band=0;band+1<rings.size();++band){const bool side=band==0;
        const std::vector<Vec3>& lower=rings[band];const std::vector<Vec3>& upper=rings[band+1];
        for(int i=0;i<key_ring_points;++i){const int j=(i+1)%key_ring_points;
            add_outward_triangle(s,side?"lens_side":"lens_round",lower[i],lower[j],upper[j],inside,side?sides:lens,false);
            add_outward_triangle(s,side?"lens_side":"lens_round",lower[i],upper[j],upper[i],inside,side?sides:lens,false);}}
    const int back_material=opaque?lens:frosted;
    const std::vector<Vec3>& base=rings.front();
    for(int i=1;i+1<key_ring_points;++i)add_outward_triangle(s,"lens_back",base[0],base[i],base[i+1],inside,back_material,
        !opaque&&!o.coverage);}
    const int last=static_cast<int>(s.primitives.size())-1;
    // The lens transmits: for light-visibility queries its faces do not
    // occlude (the few per cent lost at its boundary are neglected), so an
    // outside light's source windows are not cut by every one of its facets.
    // The camera still sees, refracts through and exits by these faces.
    for(int id=first;id<=last;++id)s.primitives[id].occludes=false;
    // The moulded top's texture: a bump on the face and its rounded edge, so
    // reflections and refraction through it are broken up as on a real
    // moulding; the sidewalls and back are left smooth.
    for(int id=first;id<=last;++id){Primitive& p=s.primitives[id];
        if(p.name=="lens_face"||p.name=="lens_top"||p.name=="lens_round"){p.relief=o.relief;p.grain=o.grain;}}

    // the legend, on the face
    const double face_h=top+.0012;
    if(opaque)add_legend(s,"CANCEL",-.04,.08,face_h,.0062,o.coverage?cover:ink);
    else add_legend(s,"OK",-.04,.08,face_h,.0062,o.coverage?cover:ink);

    if(o.coverage){build_scene_bvh(s);return s;}

    if(opaque){
        // the engine's participating medium over this polyhedron: white
        // translucent plastic; the colour is the lamp's, applied later
        Scene::ParticipatingVolume volume;volume.first=first;volume.last=last;s.jelly_volume=first;
        // a grid of hidden cores samples its source function: each is lit by
        // the lamps through the retained field, so the glow follows them
        const int white=add_material(s,{"white diffuser",MaterialKind::Diffuse,{.92,.92,.9},{},.95,.9});
        for(int j=0;j<3;++j)for(int i=0;i<5;++i){
            const double x=(i-2)*.12,y=(j-1)*.075,h=(top+bottom)*.5+.012;
            const int core=add_sphere(s,"key_core",key_point(x,y,h),.012,white,true);
            s.primitives[core].intersectable=false;volume.cores.push_back(core);}
        s.jelly_core=volume.cores[7];s.participating_volumes.push_back(volume);}
    else{
        // the clear acrylic's haze: cores along and around the coil's row, so
        // its glow falls off away from the coil
        Scene::ParticipatingVolume volume;volume.first=first;volume.last=last;s.jelly_volume=first;
        const int haze=add_material(s,{"acrylic haze",MaterialKind::Diffuse,{.92,.92,.92},{},.95,.9});
        for(int j=0;j<5;++j)for(int i=0;i<7;++i){
            const double x=(i-3)*.085,y=(j-2)*.06,h=bottom+.04+(j%2)*.03;
            const int core=add_sphere(s,"haze_core",key_point(x,y,h),.01,haze,true);
            s.primitives[core].intersectable=false;volume.cores.push_back(core);}
        s.jelly_core=volume.cores[17];s.participating_volumes.push_back(volume);}

    // Lamps. The opaque key: two long open coils at mid-depth. The clear key:
    // one long, sparse horizontal coil just above the frosted back, so its light
    // diffuses off the back, suspended in the acrylic so the faces also show it
    // reflected. Beads overlap along the wire, so it reads as one wire, not dots.
    // The clear key's coil: few, wide turns (radius .028, pitch .065).
    const double lamp_h=opaque?(top+bottom)*.5-.01:bottom+.04;
    const double coil_radius=opaque?.012:.028,pitch=opaque?.019:.065,half_length=opaque?.11:.22,bead=opaque?.0034:.0032,filament=24;
    const int hot=add_material(s,{"filament",MaterialKind::Emissive,{1,1,1},{filament,filament,filament},0,0});
    const int cold=add_material(s,{"cold filament",MaterialKind::Diffuse,{.01,.01,.01},{},.95,.9});
    const std::vector<double> lamp_rows=opaque?std::vector<double>{.075,-.075}:std::vector<double>{0.0};
    for(std::size_t lamp=0;lamp<lamp_rows.size();++lamp){const double y=lamp_rows[lamp];const bool on=o.lamp==-1||o.lamp==int(lamp);
        const int wire=on?hot:cold;
        // beads per turn so that neighbours overlap: the turn's circumference over the bead radius
        const double per_turn=std::ceil(2*pi*coil_radius/bead);const int beads=static_cast<int>(2*half_length/pitch*per_turn);
        for(int i=0;i<=beads;++i){const double x=-half_length+2*half_length*i/beads;const double a=2*pi*x/pitch;
            const int b=add_sphere(s,"filament_bead",key_point(x,y+coil_radius*std::cos(a),lamp_h+coil_radius*std::sin(a)),bead,wire,false);
            s.primitives[b].occludes=false;}  // the coil is the light: its beads must not shadow it
        if(on){
            // The coil's light, for the engine's area-light transport: a hidden
            // strip each way along the coil, of matching total power.
            const double strip=24.0*(2*pi*coil_radius)/(2*2*coil_radius);
            const int glow=add_material(s,{"coil strip",MaterialKind::Emissive,{1,1,1},{strip,strip,strip},0,0});
            // In short segments: the engine integrates an unobstructed emitter
            // with a few quadrature points along it, which a surface this close
            // (the frosted back) would see as separate hot spots.
            const int segments=std::max(1,static_cast<int>(std::ceil(2*half_length/.02)));
            for(int piece=0;piece<segments;++piece)for(int side=0;side<2;++side){const double sign=side?-1:1;
                const double x0=-half_length+2*half_length*piece/segments;
                const Vec3 origin=key_point(x0,y-coil_radius,lamp_h);
                const int r=add_rect(s,"coil_strip",origin,key_point(2*half_length/segments,0,0),key_point(0,2*coil_radius,0),{0,sign,0},glow,false);
                s.primitives[r].intersectable=false;s.area_lights.push_back({r,s.materials[glow].emission});}}}
    if(o.fill){
        // The room, all outside the camera's view. A strong white softbox above
        // and behind the key, laid as strips fading to nothing towards its near
        // edge, so its reflection in the key's top and chamfer is a soft
        // highlight with no hard edge. Dimmer panels left, right and in front
        // light the plastic from around the room and give the chamfers soft
        // side reflections.
        const int strips=10;
        for(int i=0;i<strips;++i){const double t0=double(i)/strips,t1=double(i+1)/strips;
            const double fade=std::pow(1-(t0+t1)/2,1.5);  // 1 at the far edge, 0 at the near one
            const double radiance=3.2*fade;
            const int box=add_material(s,{"softbox",MaterialKind::Emissive,{1,1,1},{radiance,radiance,radiance},0,0});
            const Vec3 far{-0.8,1.32,-0.9},depth{0,.18,.75};
            const int r=add_rect(s,"softbox",far+depth*t0,{1.6,0,0},depth*(t1-t0),unit(Vec3{0,-1.2,.288}),box,false);
            s.area_lights.push_back({r,s.materials[box].emission});}
        const int side=add_material(s,{"room",MaterialKind::Emissive,{1,1,1},{.9,.9,.92},0,0});
        const std::array<std::array<Vec3,4>,3> panels{{
            {Vec3{-1.3,0,-1.0},Vec3{0,0,2.0},Vec3{0,1.4,0},unit(Vec3{1,-.3,0})},    // left
            {Vec3{1.3,0,1.0},Vec3{0,0,-2.0},Vec3{0,1.4,0},unit(Vec3{-1,-.3,0})},    // right
            {Vec3{1.0,0,1.4},Vec3{-2.0,0,0},Vec3{0,1.0,0},unit(Vec3{0,-.2,-1})}}};  // front, below the camera's view
        for(const auto& p:panels){const int r=add_rect(s,"room_panel",p[0],p[1],p[2],p[3],side,false);
            s.area_lights.push_back({r,s.materials[side].emission});}}
    build_scene_bvh(s);return s;}

