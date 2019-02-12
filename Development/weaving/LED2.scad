
// Length of insulating bead in mm
k   = 9.0;
// Extra length padding for wire
ep  = 0.5;
// Total spacing between OUTER edges
// K = 2*(rs + rh*2 + rc)
K   = k+ep;

// Diameter and radius of threading hole
dh  = 0.9;
rh  = dh/2;

// Plated padding radius
rc  = dh*0.9;

// Hole separation radius
rs  = K/2 - dh - rc;

rd  = rs + rc + rh;
rC  = rh+rc;
w   = 2*rC;
l   = rd+1e-9;
pad = 0.8;

module arm(){    
    difference() {
        translate([-rC,0,0]) 
        square(size = [w, l], center = false);
        translate([0,rd,0])  
        circle(r=rh,$fn=100);
    }
    translate([0,rd,0]) 
    difference() {
        circle(r=rc+rh,$fn=100);
        circle(r=rh,$fn=100);
    }
}  

// bridge btwn connected LEDs
bfudge = 0.12;
blen   = pad + 1e-6 + bfudge*2;
bwidth = blen+0.4;
module bridge() {
    difference() {
        square(size = [blen, bwidth], center = false);
        translate([blen/2,0,0]) 
            circle(r=blen/2,$fn=100);
        translate([blen/2,bwidth,0]) 
            circle(r=blen/2,$fn=100);
        //translate([blen/2,bwidth/2,0]) 
        //    circle(r=blen/2,$fn=100);
    }
}


re = rh+rc+pad;

module led(){  
    for(h = [0,90,180,270]) rotate(h) arm();

    mirror([1,0,0])
    rotate(90)
    translate([w/2-bfudge,rs+rc-blen+0.6,0])
    bridge();

    rotate(0)
    mirror([1,0,0])
    translate([w/2-bfudge,rs+rc-blen+0.6,0])
    bridge();

    difference () {
    square(size=[w+2*re,w+2*re],center=true);
        for(h = [0,90,180,270]) 
            rotate(h)
                translate([w/2+re,w/2+re])
                circle(r=re,$fn=100);
    }
}



dx = l+pad+w;
dy = w+pad;
dy2 = l+w+pad;
dx2 = -w-pad;

theta = atan2(dx,dy);
hypot = sqrt(dx*dx+dy*dy);

//led();

difference() {
    union() {
        rotate(theta+45) {
        for(j=[-7:7]) for(i=[-7:7]) 
            translate([dx2*j,dy2*j,0]) 
            translate([dx*i,dy*i,0]) 
                led();
        }
        // Edge supports
        
        translate([-52.5,0])
        square([10,80],center=true);
        translate([52.5,0])
        square([10,80],center=true);
        translate([0,43.75])
        square([100,10],center=true);
        translate([0,-43.75])
        square([100,10],center=true);
        
        
        // Edge bridges
        translate([-39.25-hypot,0])
        for(i=[-4:4])
            translate([0,1+i*hypot])
                bridge();
    }
    // Trim edges (intersection doesn't work?)
    translate([50,-500,0])
    square(size=[200,1000],center=false);
    translate([-500,40,0])
    square(size=[1000,200],center=false);
    rotate(180) {
        translate([50,-500,0])
        square(size=[200,1000],center=false);
        translate([-500,40,0])
        square(size=[1000,200],center=false);
    }
}