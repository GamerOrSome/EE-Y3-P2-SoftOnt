int xpos =100;
int ypos = 50;
int rec_width = 200;
int rec_height = 70;
int spacer = 80;
int spacer2 = 210;
int rounded = 10;
int stage = 0;
char coord[];
boolean [] toggle;
boolean lock;
PFont myFont;

void setup()
{
  size(720, 720); //defines application screen size
  background(#78BEED);
  for(int i = 0; i < 5; i++){
    fill(255, 0, 0);
    rect(xpos, ypos + (i * spacer), rec_width, rec_height, rounded);
    if(i < 4){
      for(int y = 0; y < 2; y++){
        fill(255, 0, 0);
        rect(xpos + (y * spacer2), ypos + (i * spacer), rec_width, rec_height, rounded);
      }
    }
  }
  noSmooth();
  toggle = new boolean[9];
  coord = new char[4];
}

void draw()
{
  if(toggle[1] == true){
    background(#5599FF);
    fill(0);
    textSize(49);
    text("lijn X1 Y1", 200, 300, 200, 200);
    fill(255);
    rect(200, 350, 200, 30, 5);
    stage = 1;
    if(stage == 1){
      rect(10, 10, 10, 10, 1);
      if(keyPressed && key == '1'){
        stage = 2;
      }
    }
    if(stage == 2){
      myFont = createFont("arial" , 9);
      fill(0);
      textFont(myFont);
      textSize(20);
      text(coord, 1, 2, 200, 400);
      rect(50, 10, 10, 10, 1);
    }
  }else if(toggle[2] == true){
    background(#5599FF);
    fill(0);
    textSize(50);
    text("cirkel X1 Y1", 200, 300, 250, 200);
    fill(255);
    rect(200, 350, 200, 30, 5);
  }else{
    background(#78BEED);
    for(int i = 0; i < 5; i++){
      fill(255, 0, 0);
      rect(xpos, ypos + (i * spacer), rec_width, rec_height, rounded);
      if(i < 4){
        for(int y = 0; y < 2; y++){
          fill(255, 0, 0);
          rect(xpos + (y * spacer2), ypos + (i * spacer), rec_width, rec_height, rounded);
        }
      }
    }
    fill(0, 0, 0);
    textSize(50);
    text("lijn", 170, 65, 200, 200);
    text("cirkel", 355, 65, 200, 200);
  }    
}
  

void mousePressed()
{
  if(lock == false){
    if(mouseX > xpos && mouseX < xpos + rec_width && mouseY > ypos && mouseY < ypos + rec_height){
      toggle[1] = true;
      lock = true;
    }
    if(mouseX > xpos + spacer2 && mouseX < xpos + rec_width + spacer2 && mouseY > ypos && mouseY < ypos + rec_height){
      toggle[2] = true;
      lock = true;
    }
  }
}

void keyPressed() {
  if(key == BACKSPACE){
    for(int i = 0; i < 9; i++){
      toggle[i] = false;
      lock = false;
    }
  }
}
  
