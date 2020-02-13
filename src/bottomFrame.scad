
$fn=100;
length = 210;
height = 12;
width = 75;
thick = 3;

intersection() {
    cube([length, width, height]);
    translate([5,5,0]) ring(24, 12, 3);
}

intersection() {
    cube([length, width, height]);
    translate([length - 5,5,0]) ring(24, 12, 3);
}

intersection() {
    cube([length, width, height]);
    translate([5,width - 5,0]) ring(24, 12, 3);
}

intersection() {
    cube([length, width, height]);
    translate([length - 5,width - 5,0]) ring(24, 12, 3);
}

difference() {
    cube([length, width, height]);
    translate([thick, thick, 0]) cube([length - 2*thick, width - 2*thick, height]);
    translate([5,5,0]) cylinder(h=height, d=24);
    translate([length - 5,width - 5,0]) cylinder(h=height, d=24);
    translate([5,width - 5,0]) cylinder(h=height, d=24);
    translate([length - 5,5,0]) cylinder(h=height, d=24);
    translate([length/2, width/2, height/2]) cube([50,width, height], true);
}

translate([length/2+13, 5, 0]) sector(30, 12, 3);
translate([length/2-13, 5, 0]) rotate([0, 0, 90]) sector(30, 12, 3);
translate([length/2+13, width - 5, 0]) rotate([0, 0, 270]) sector(30, 12, 3);
translate([length/2-13, width - 5, 0]) rotate([0, 0, 180]) sector(30, 12, 3);

translate([length/2+25, 5/2, 0]) cube([3, 3, height]);
translate([length/2-28, 5/2, 0]) cube([3, 3, height]);
translate([length/2+25, width - 5, 0]) cube([3, 3, height]);
translate([length/2-28, width - 5, 0]) cube([3, 3, height]);

translate([length/2-13, 17, 0]) cube([26, 3, height]);
translate([length/2-13, width - 20, 0]) cube([26, 3, height]);

module sector(diam, height, thick) {
    intersection() {
        cube([diam, width, height]);
        ring(diam, height, thick);
    }
}

module ring(diam, height, thick) {
    difference() {
        cylinder(h=height, r=diam/2);
        cylinder(h=height, r=(diam/2-thick));
    }
}