
$fn=100;

length = 85;
height = 11;

diam = 2 * height;
width= 3 * height;

baseH=0.4;

btnOuter = 21;
btnInner = 17;
btnOff = 87.5;

stipOff=3;
stipWidth=13;
stripLength = 75;
stripHeight=4;

difference() {
    union() {
        hull() {
            union(){
                translate([diam/2, 0, 0])sphere(d=diam);
        //            translate([length-diam, 0, 0]) sphere(d=diam);
                translate([btnOff-1, 0, 0]) rotate([0, 90, 0]) cylinder(d=diam, h=1);
            }
            translate([diam/2, -width/2, 0]) {
                // cube([length + diam, 4* diam, 1]);
                linear_extrude(height=baseH) {
                    offset(r=diam/2) square([length-diam, width]);
                }
            }
        }
        translate([btnOff, 0, 0]) cylinder(d=btnOuter, h=height);
    }
    translate([btnOff, 0, 0]) cylinder(d=btnInner, h=height);
    translate([-1, -width, -height]) cube([2*length+2, 2*width, height]);
    translate([stipOff, -stipWidth/2, 0]) cube([stripLength, stipWidth, stripHeight]);
}
