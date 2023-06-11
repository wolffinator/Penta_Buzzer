$fa = 1;
$fn=80;

displayTop = 1;


batteryLength = 31;
batteryHeight = 9;
batteryWidth = 20;

batteryDividerThickness = 0.4;

MCUBoardLength = 20;
MCUBoardWidth = 18;
MCUComboBoardHeight = 7.8;

usbPortWidth = 9.5;
usbPortHeight = 3.5;



boardThickness = 1.5;
boardThicknessWiggle = 0.05;
boardLength = 60;
boardWidth = 23;
boardWidthLengthWiggle = 0.5;
outerWallThickness = 1.2;

topWallThickness = 1.32;

buttonCutoutDiameter = 6.45;


buttonHeight = 4.3;
pushButtonHeightFromStopper = 3.5;

pushButtonRodHeight = 1.5;
buttonWiggleRoom = 0.05;
heightFromBoardToBaseofInnerWall = buttonHeight + pushButtonRodHeight + buttonWiggleRoom;
heightFromBoardToBaseofInnerBottomWall = 10;
//heightFromBoardToZBottomOfTop = buttonHeight + pushButtonHeightFromStopper + 0.36 + 5;
heightFromBoardToZBottomOfTop = 14.5;

screwHoleOffsets = 2.5;
screwHoleDiameter = 1.98;
screwHeightFromHead = 5.5;
screwHoleOuterWallThickness = 1.8;


MCUOffCenter = -2.5;

botxyLeftEdge =  [ boardWidth/2, -boardLength/2];
botxyRightEdge = [ boardWidth/2,  boardLength/2];
topxyLeftEdge =  [-boardWidth/2, -boardLength/2];
topxyRightEdge = [-boardWidth/2,  boardLength/2];
button1Pos =     [0 + MCUOffCenter, botxyRightEdge[1]-10];

zTopBoard = boardThickness/2;

batteryFrontY = -(boardLength / 2) + batteryLength;


ribbingSeperation = 4;


bottomWallThickness = 1;
bottomWallThickness2 = 0.5;
bottomWallInsetWiggle = 0.45;



//translate([0,batteryFrontY - (batteryLength/2),batteryHeight/2])
//cube([batteryWidth, batteryLength, batteryHeight], center = true);

//translate([0, boardLength/2 - MCUBoardLength/2,MCUComboBoardHeight/2])
//cube([MCUBoardWidth, MCUBoardLength, MCUComboBoardHeight], center = true);




module batteryDivider(){
    difference(){
        translate([0,batteryFrontY + batteryDividerThickness/2,heightFromBoardToZBottomOfTop/2])
            cube([boardWidth, batteryDividerThickness, heightFromBoardToZBottomOfTop], center = true);
       translate([boardWidth/2 - 15/2,0,0])
            cube([15,20,10], center = true); 
    }
}

module PerfBoard(){
    difference(){
        cube([boardWidth, boardLength, boardThickness], center = true);
        translate([botxyLeftEdge[0] - screwHoleOffsets,botxyLeftEdge[1] + screwHoleOffsets,0])
        cylinder(h=boardThickness*2, d = screwHoleDiameter, center = true);
        translate([botxyRightEdge[0] - screwHoleOffsets,botxyRightEdge[1] - screwHoleOffsets,0])
        cylinder(h=boardThickness*2, d = screwHoleDiameter, center = true);
        translate([topxyLeftEdge[0] + screwHoleOffsets,topxyLeftEdge[1] + screwHoleOffsets,0])
        cylinder(h=boardThickness*2, d = screwHoleDiameter, center = true);
        translate([topxyRightEdge[0] + screwHoleOffsets,topxyRightEdge[1] - screwHoleOffsets,0])
        cylinder(h=boardThickness*2, d = screwHoleDiameter, center = true);
    }
}
module ScrewStandoff(){
    difference(){
        cylinder(h=screwHeightFromHead,d = screwHoleDiameter + screwHoleOuterWallThickness);
        cylinder(h=screwHeightFromHead,d = screwHoleDiameter);
    };
    translate([0,0,screwHeightFromHead])
        cylinder(h=heightFromBoardToZBottomOfTop - screwHeightFromHead-2,d = screwHoleDiameter + screwHoleOuterWallThickness);
}

module PushButtonRotationSetterPair(){
    translate([0,0,-0.5])
    difference(){
        cube([4,9.8,2], center = true);
        cube([1.4,100,100], center = true);
        cube([100,6.5,100], center = true);
    }
}
module PushButtonRotationSetters(){
    rotate([0,0,90])
    difference(){
        rotate([0,0,90])
        PushButtonRotationSetterPair(); 
        translate([50,0,0])
        cube([100,10,10], center = true);
    }
}


module BoardTop(){
    difference(){
        cube([boardWidth, boardLength, topWallThickness], center = true);
        translate([button1Pos[0],button1Pos[1],0])
            cylinder(h = 50, d = 6.45, center = true);
    }
    translate([button1Pos[0],button1Pos[1],0])
        PushButtonRotationSetters();

    
}

module outerWalls()
{
    translate([0,boardLength/2 + outerWallThickness/2,heightFromBoardToZBottomOfTop/2 + topWallThickness/2])
        cube([boardWidth,outerWallThickness,heightFromBoardToZBottomOfTop + topWallThickness], center = true);
    translate([0,-boardLength/2 - outerWallThickness/2,heightFromBoardToZBottomOfTop/2 + topWallThickness/2])
        cube([boardWidth,outerWallThickness,heightFromBoardToZBottomOfTop + topWallThickness], center = true);
    
    translate([-boardWidth/2 - outerWallThickness/2,0,heightFromBoardToZBottomOfTop/2 + topWallThickness/2])
    rotate([0,0,90])
        cube([boardLength + outerWallThickness*2,outerWallThickness,heightFromBoardToZBottomOfTop + topWallThickness], center = true);
    translate([boardWidth/2 + outerWallThickness/2,0,(heightFromBoardToZBottomOfTop/2) + (topWallThickness/2)])
    rotate([0,0,90])
        cube([boardLength + outerWallThickness*2,outerWallThickness,heightFromBoardToZBottomOfTop + topWallThickness], center = true);
    for(i = [-boardLength/2: ribbingSeperation: boardLength/2]){
    translate([boardWidth/2,i,heightFromBoardToZBottomOfTop/2])
    ribbing(heightFromBoardToZBottomOfTop,2);
    }
    
    rotate([0,0,180])
    translate([-boardWidth/2,0,9])
    rotate([90,180,0])
    ribbing(boardLength,2);
}
module ribbing(height, rib_diameter){
difference(){
    cylinder(h=height, d = rib_diameter, center = true);
    translate([rib_diameter/2,0,0])
    cube([rib_diameter,rib_diameter+1,height+1], center = true);
}
}
module CaseTop()
{
    //PerfBoard();
    //translate([botxyLeftEdge[0] - screwHoleOffsets,botxyLeftEdge[1] + screwHoleOffsets,0])
    //    ScrewStandoff();
    translate([0 + MCUOffCenter,batteryFrontY + batteryDividerThickness + screwHoleOuterWallThickness,3])
        ScrewStandoff();
    //translate([topxyLeftEdge[0] + screwHoleOffsets,topxyLeftEdge[1] + screwHoleOffsets,0])
    //    ScrewStandoff();
    //translate([topxyRightEdge[0] + screwHoleOffsets,topxyRightEdge[1] - screwHoleOffsets,0])
    //    ScrewStandoff();
    translate([0,0,heightFromBoardToZBottomOfTop+topWallThickness/2])
        BoardTop();
    //translate([0,0,0])
    //batteryDivider();
    difference(){
        outerWalls();
        translate([0 + MCUOffCenter,40,8.6])
        cube([usbPortWidth+0.2, 40,usbPortHeight+0.8], center = true);
        for(i = [0:1:3]){
            translate([9,4 + i*ribbingSeperation * 2,5])
            rotate([0,90,0])
            cylinder(h=10, d = 2, center = true);
        }
        for(i = [0:1:3]){
            translate([9,4 + i*ribbingSeperation * 2,13])
            rotate([0,90,0])
            cylinder(h=10, d = 2, center = true);
        }
    }

    outerWallPeg = 5.5;
    translate([0 + MCUOffCenter,7,7.5])
    cube([1,4.8,5], center = true);
    translate([14.25-(outerWallPeg/2) + MCUOffCenter,18,7.5])
    rotate([0,0,90])
    cube([1,outerWallPeg,5], center = true);

    //translate([-12.25+(outerWallPeg/2) + MCUOffCenter,18,7.5])
    //rotate([0,0,90])
    //cube([1,outerWallPeg,5], center = true);

    difference(){
        testVar = 5.0;
        translate([button1Pos[0],button1Pos[1],heightFromBoardToZBottomOfTop - testVar/2])
        cylinder(h=testVar, d = 11, center = true);
        translate([button1Pos[0],button1Pos[1],heightFromBoardToZBottomOfTop - testVar/2])
        cylinder(h=testVar, d = 9.2, center = true);
        translate([button1Pos[0],button1Pos[1] - 5,heightFromBoardToZBottomOfTop - 5])
            cube([4,2,10], center = true);
        translate([button1Pos[0],button1Pos[1] + 5,heightFromBoardToZBottomOfTop - 5])
            cube([14,13,10], center = true);
    }
}
module CaseBottom()
{
    translate([0 + MCUOffCenter,batteryFrontY + batteryDividerThickness + screwHoleOuterWallThickness,0])
    difference(){
        cylinder(h=3 - 0.34,d = screwHoleDiameter + screwHoleOuterWallThickness + 2);
        cylinder(h=screwHeightFromHead,d = screwHoleDiameter + 2.75);
    };
    difference(){
        union(){
            translate([0,0,-bottomWallThickness2/2]);
            cube([boardWidth+(outerWallThickness*2), boardLength+(outerWallThickness*2), bottomWallThickness2], center = true);
            difference()
            {
                translate([0,0,bottomWallThickness/2])
                cube([boardWidth - bottomWallInsetWiggle, boardLength - bottomWallInsetWiggle, bottomWallThickness], center = true);
                    for(i = [-boardLength/2: ribbingSeperation: boardLength/2]){
                        translate([boardWidth/2+0.01,i,heightFromBoardToZBottomOfTop/2 - 4])
                        resize([2, 2.6, 0])
                            ribbing(heightFromBoardToZBottomOfTop,2);
                }
            }
        }
    translate([0 + MCUOffCenter,batteryFrontY + batteryDividerThickness + screwHoleOuterWallThickness,-3])
    cylinder(h=screwHeightFromHead,d = screwHoleDiameter + 2.75);
    }
    translate([0 + MCUOffCenter,batteryFrontY + batteryDividerThickness + screwHoleOuterWallThickness,2])
    difference(){
        cylinder(h=1 - 0.34,d = screwHoleDiameter + screwHoleOuterWallThickness + 2);
        cylinder(h=1 - 0.34,d = screwHoleDiameter);
    };
    translate([0 + MCUOffCenter,batteryFrontY + batteryDividerThickness + screwHoleOuterWallThickness,2])
    rotate([180,0,0])
    fannout(2.9);
}
module fannout(diameter)
{
    
rotate_extrude()
polygon([[0+diameter,0],[0+diameter,1],[1+diameter,1], [0.7+diameter,0.9], [0.5+diameter,0.75], [0.3+diameter,0.55], [0.15+diameter,0.35]]);
}

CaseTop();
translate([30,0,0])
CaseBottom();