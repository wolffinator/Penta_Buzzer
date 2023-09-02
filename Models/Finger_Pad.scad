$fn=30;
fingerWidth=14;
miniOuterCircleD = fingerWidth/3;
fingerPadLength=18;
linear_extrude(10){
    difference(){
        union(){
            translate([fingerPadLength/2,0,0])
            square([fingerPadLength,fingerWidth], center = true);
            circle(d=fingerWidth);
            translate([fingerPadLength,fingerWidth/2-miniOuterCircleD/2,0])
            circle(d=miniOuterCircleD);
            translate([fingerPadLength,-fingerWidth/2+miniOuterCircleD/2,0])
            circle(d=miniOuterCircleD); 
        }
        translate([fingerPadLength+((fingerPadLength*4.5)/10),0,0])
        circle(d=fingerWidth+3);
    }
}