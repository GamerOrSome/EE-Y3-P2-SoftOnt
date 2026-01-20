import processing.serial.*;

Serial myPort;

int xpos =100;         //xpos for boxes
int ypos = 50;         //ypos for boxes
int rec_width = 200;   //width of the boxes
int rec_height = 70;   //height of the boxes
int spacer = 80;       //space inbetween the boxes vertical
int spacer2 = 210;     //space between de boxes horizontally
int rounded = 10;      //radius of the rounded edges of the boxes
int stage = 0; 
int coord[];
int coord_stage = 0;
boolean [] toggle;
boolean lock;
PFont myFont;

void setup()
{
  size(850, 720); //defines application screen size
  
  myFont = createFont("arial" , 9); //creates custom font which allows numbers to be printed
  textFont(myFont);
  
  myPort = new Serial(this, "COM10", 115200);
  
  toggle = new boolean[9];
  coord = new int[8];
}

void draw()
{
  if(toggle[0] == true)
  {
    switch(coord_stage)
    {
      case 0:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("lijn X1", 170, 300, 200, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 1:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("lijn Y1", 170, 300, 200, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 2:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("lijn X2", 170, 300, 200, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 3:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("lijn Y2", 170, 300, 200, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 4:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("lijn kleur", 170, 300, 200, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 5:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("lijn dikte", 170, 300, 200, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 6:
        String msg = "lijn," + coord[0] + "," + coord[1] + "," + coord[2] + "," + coord[3] + "," + "rood" + "," + coord[5] + "\n\r";
        print(msg);
        myPort.write(msg);
        coord_stage = 0;
        toggle[0] = false;
        lock = false;
        break;
    }
  }
  else if(toggle[1] == true)
  {
    switch(coord_stage)
    {
      case 0:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("rechthoek X1", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 1:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("rechthoek Y1", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 2:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("rechthoek X2", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 3:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("rechthoek Y2", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 4:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("rechthoek kleur", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 5:
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("rechthoek filled:  1 for yes, 0 for no", 170, 250, 315, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 6:
        String msg = "recht," + coord[0] + "," + coord[1] + "," + coord[2] + "," + coord[3] + "," + "blauw" + "," + coord[5] + "\n\r";
        print(msg);
        myPort.write(msg);
        coord_stage = 0;
        toggle[0] = false;
        lock = false;
        break;
    }
  }
  if(lock == false)
  {
    background(#78BEED);
    for(int i = 0; i < 3; i++)
    {
      fill(255, 0, 0);
      rect(xpos, ypos + (i * spacer), rec_width, rec_height, rounded);
      if(i < 4)
      {
        for(int y = 0; y < 3; y++)
        {
          fill(255, 0, 0);
          rect(xpos + (y * spacer2), ypos + (i * spacer), rec_width, rec_height, rounded);
        }
      }
    }
    fill(0, 0, 0);
    textSize(50);
    text("lijn", 170, 65, 200, 200);
    textSize(30);
    text("rechthoek", 350, 70, 200, 200);
    textSize(50);
    for(int i = 0; i < 4; i++)
    {
      coord[i] = 0;
    }
  }
}
  

void mousePressed()
{
  if(lock == false){
    if(mouseX > xpos && mouseX < xpos + rec_width && mouseY > ypos && mouseY < ypos + rec_height) //checks if the line box has been pressed
    {
      toggle[0] = true;
      lock = true;
    }
    if(mouseX > xpos + spacer2 && mouseX < xpos + rec_width + spacer2 && mouseY > ypos && mouseY < ypos + rec_height) //checks the circle box 
    {
      toggle[1] = true;
      lock = true;
    }
  }
}

void keyPressed() {
  if(key == BACKSPACE)
  {
    for(int i = 0; i < 9; i++)
    {
      toggle[i] = false;
      lock = false;
    }
  }
  if(key == ENTER)
  {
    coord_stage = coord_stage + 1;
  }
  if(key >= '0' && key <= '9')
  {
    int digit = key - '0';      
    coord[coord_stage] = coord[coord_stage] * 10 + digit;
  }
}
  
