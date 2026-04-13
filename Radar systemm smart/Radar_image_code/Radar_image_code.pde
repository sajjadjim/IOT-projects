// ==========================================
// RADAR SIMULATION MODE (NO ARDUINO NEEDED)
// ==========================================

int iAngle = 15;
int iDistance = 100;
int direction = 2; // Speed of the sweep

void setup() {
  size(1200, 700); // Size of the window
  smooth();
  // Notice: No Serial port code here!
}

void draw() {
  // --- 1. SIMULATE THE SWEEPING MOTION ---
  iAngle += direction;
  if (iAngle >= 165 || iAngle <= 15) {
    direction *= -1; // Reverse direction when it hits the edges
  }

  // --- 2. SIMULATE FAKE OBJECTS ---
  // Pretend there is an object right in the middle (~90 degrees)
  if (iAngle > 85 && iAngle < 95) {
    iDistance = (int) random(25, 35); // Object is about 30cm away
  } 
  // Pretend there is an object on the left side (~140 degrees)
  else if (iAngle > 135 && iAngle < 145) {
    iDistance = 15; // Object is 15cm away
  } 
  // Everywhere else is clear
  else {
    iDistance = 100; // Out of range
  }

  // --- 3. DRAW THE RADAR SCREEN ---
  // Fading motion blur effect
  noStroke();
  fill(0, 7); 
  rect(0, 0, width, height - height*0.065); 
  
  drawRadar(); 
  drawLine();
  drawObject();
  drawText();
  
  delay(20); // Controls how fast the simulation runs
}

// Draws the green semi-circles and grid lines
void drawRadar() {
  pushMatrix();
  translate(width/2, height-height*0.074); 
  noFill();
  strokeWeight(2);
  stroke(98, 245, 31); // Neon Green
  
  arc(0, 0, (width-width*0.0625), (width-width*0.0625), PI, TWO_PI);
  arc(0, 0, (width-width*0.27), (width-width*0.27), PI, TWO_PI);
  arc(0, 0, (width-width*0.479), (width-width*0.479), PI, TWO_PI);
  arc(0, 0, (width-width*0.687), (width-width*0.687), PI, TWO_PI);
  
  line(-width/2, 0, width/2, 0);
  line(0, 0, -width/2*cos(radians(30)), -width/2*sin(radians(30)));
  line(0, 0, -width/2*cos(radians(60)), -width/2*sin(radians(60)));
  line(0, 0, -width/2*cos(radians(90)), -width/2*sin(radians(90)));
  line(0, 0, -width/2*cos(radians(120)), -width/2*sin(radians(120)));
  line(0, 0, -width/2*cos(radians(150)), -width/2*sin(radians(150)));
  popMatrix();
}

// Draws a RED line if an object is closer than 40cm
void drawObject() {
  pushMatrix();
  translate(width/2, height-height*0.074);
  strokeWeight(9);
  stroke(255, 10, 10); // Red
  
  if (iDistance < 40) {
    float pixsDistance = iDistance * ((height-height*0.1666)*0.025); 
    line(pixsDistance*cos(radians(iAngle)), -pixsDistance*sin(radians(iAngle)), 
         (width-width*0.505)*cos(radians(iAngle)), -(width-width*0.505)*sin(radians(iAngle)));
  }
  popMatrix();
}

// Draws the sweeping scanning line
void drawLine() {
  pushMatrix();
  strokeWeight(9);
  stroke(30, 250, 60); // Bright Green
  translate(width/2, height-height*0.074); 
  line(0, 0, (height-height*0.12)*cos(radians(iAngle)), -(height-height*0.12)*sin(radians(iAngle))); 
  popMatrix();
}

// Draws the text data at the bottom of the screen
void drawText() {
  pushMatrix();
  fill(0, 0, 0);
  noStroke();
  rect(0, height-height*0.0648, width, height);
  
  fill(98, 245, 31);
  textSize(25);
  text("10cm", width-width*0.3854, height-height*0.0833);
  text("20cm", width-width*0.281, height-height*0.0833);
  text("30cm", width-width*0.177, height-height*0.0833);
  text("40cm", width-width*0.0729, height-height*0.0833);
  
  textSize(40);
  text("Angle: " + iAngle +" °", width-width*0.48, height-height*0.0277);
  text("Distance: ", width-width*0.26, height-height*0.0277);
  if (iDistance < 40) {
    text("        " + iDistance +" cm", width-width*0.225, height-height*0.0277);
  } else {
    text("        Out of Range", width-width*0.225, height-height*0.0277);
  }
  popMatrix();
}
