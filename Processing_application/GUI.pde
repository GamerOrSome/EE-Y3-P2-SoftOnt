import processing.serial.*;  //import de library die nodig is voor seriele communicatie

Serial myPort;          //wordt gebruikt voor het aanmaken van een serial port

int xpos =100;          //xpos for boxes
int ypos = 50;          //ypos for boxes
int rec_width = 200;    //width of the boxes
int rec_height = 70;    //height of the boxes
int spacer = 80;        //space inbetween the boxes vertical
int spacer2 = 210;      //space between de boxes horizontally
int rounded = 10;       //radius of the rounded edges of the boxes

int coord[];            //array die coordinaten data opslaat
int coord_stage = 0;    //wordt gebruikt om te kijken in welke stage de code is  
boolean [] toggle;      //array die kijkt welk commando momenteel aanstaat
boolean lock;           //boolean om die gebruikt wordt om de input van toggle to locken

PFont myFont;           //costom font in processing code

String kleur = "";      //string voor het opslaan van de gekozen kleur
String tekst = "";      //string voor het opslaan van de gekozen tekst
String fontnaam = "";   //string voor het opslaan van de gekozen font
String fontstijl = "";  //string voor het opslaan van de gekozen fontstijl


void setup()
{
  size(850, 720); //defineerd de window size van de applicatie
  
  myFont = createFont("arial" , 9); //maakt de custom font aan zodat er nummers geprint kunnen worden
  textFont(myFont);
  
  myPort = new Serial(this, "com11", 115200); //initialissert de seriele port
  
  toggle = new boolean[5];    //hier worden de hoeveelheid commandos geinitaliseerd kan vergroot worden indien meer gewenste commandos
  coord = new int[7];         //geeft aan hoeveel posities erzijn binnen de coord array indien er meer dan 7 posities aan coordinaten en dergelijk nodig zijn moet dit vergroot worden
  for(int i = 0; i < 7; i++)  //zet alle posities van het array op 0 om mogelijke null pointer errors te voorkomen
  {
    coord[i] = 0;
  }
}

void draw()
{
  if(lock == false)              //kijkt of de lock waarde false is en of er geen commandos geselcteerd zijn
  {
    background(#78BEED);         //zorgt dat de achtergrond blauw gekleurd is

    for(int i = 0; i < 5; i++)   //deze for loop zorgt dat alle rechthoeken op het scherm worden getekend
    {
      fill(255, 0, 0);
      rect(xpos, ypos + (i * spacer), rec_width, rec_height, rounded);
    }
    
    //tekent de tekst in de rechthoeken
    fill(0, 0, 0);
    textSize(50);
    text("lijn", 170, 65, 200, 200);    
    textSize(30);
    text("rechthoek", 135, 152, 200, 200);
    textSize(50);
    text("tekst", 150, 225, 200, 200);
    text("bitmap", 130, 300, 200, 200);
    text("clear", 150, 380, 200, 200);

    for(int i = 0; i < 7; i++)   //cleared de waardes van zowel de coord array en de strings
    {
      coord[i] = 0;
    }
    kleur = "";
    tekst = "";
    fontnaam = "";
    fontstijl = "";
  }

  if(toggle[0] == true)
  {
    switch(coord_stage)
    {
      case 0: //lijn X1
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
      case 1: //lijn Y1
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
      case 2: //lijn X2
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
      case 3: //lijn Y2
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
      case 4: //lijn kleur
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("lijn kleur", 170, 300, 200, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(kleur, 180, 373);
        break;
      case 5: //lijn dikte
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
      case 6: //verzenden lijn data
        String msg = "lijn," + coord[0] + "," + coord[1] + "," + coord[2] + "," + coord[3] + "," + kleur + "," + coord[5] + "\r\n";  //voegt alle waardes samen naar 1 string
        myPort.write(msg);  //stuurt de string via de seriele port
        println(msg);         //print de string voor debugging
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
      case 0: //rechthoek X1
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
      case 1: //rechthoek Y1
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
      case 2: //rechthoek X2
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
      case 3: //rechthoek Y2
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
      case 4: //rechthoek kleur
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("rechthoek kleur", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(kleur, 180, 373);
        break;
      case 5: //rechthoek filled
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
      case 6: //verzenden rechthoek data
        String msg = "rechthoek," + coord[0] + "," + coord[1] + "," + coord[2] + "," + coord[3] + "," + kleur + "," + coord[5] + "\r\n";
        myPort.write(msg);
        println(msg);
        coord_stage = 0;
        toggle[1] = false;
        lock = false;
        break;
    }
  }else if(toggle[2] == true)
  {
    switch(coord_stage)
    {
      case 0: //tekst X1
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("tekst X1", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 1: //tekst Y1
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("tekst Y1", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 2: //tekst kleur
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("kleur", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(kleur, 180, 373);
        break;
      case 3: //tekst inhoud
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("tekst", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(tekst, 180, 373);
        break;
      case 4: //tekst font
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("font", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(fontnaam, 180, 373);
        break;
      case 5: //tekst fontsize
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("fontsize", 170, 300, 315, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 6: //tekst fontstijl
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("fontstijl", 170, 300, 315, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(fontstijl, 180, 373);
        break;
      case 7: //verzenden tekst data
        String msg = "tekst," + coord[0] + "," + coord[1] + "," + kleur + "," + tekst + "," + fontnaam + "," + coord[5] + "," + fontstijl + "\r\n";
        myPort.write(msg);
        println(msg);
        coord_stage = 0;
        toggle[2] = false;
        lock = false;
        break;
    }
  }else if(toggle[3] == true)
  {
    switch(coord_stage)
    {
      case 0: //bitmap nr
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("bitmap nr", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 1: //bitmap X pos
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("X pos", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 2: //bitmap Y pos
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("Y pos", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(coord[coord_stage], 180, 373);
        break;
      case 3: //verzenden bitmap data
        String msg = "bitmap," + coord[0] + "," + coord[1] + "," + coord[2] + "," + "\r\n";
        myPort.write(msg);
        println(msg);
        coord_stage = 0;
        toggle[3] = false;
        lock = false;
        break;
    }
  }else if(toggle[4] == true)
  {
    switch(coord_stage)
    {
      case 0: //clearscherm kleur
        background(#0078DE);
        fill(#000000);
        textSize(40);
        text("kleur", 170, 300, 300, 200);
        fill(#FFFFFF);
        rect(170, 350, 200, 30, 5);
        fill(#000000);
        textSize(20);
        text(kleur, 180, 373);
        break;
      case 1: //verzenden clearscherm data
        String msg = "clearscherm," + kleur + "\r\n";
        myPort.write(msg);
        println(msg);
        coord_stage = 0;
        toggle[4] = false;
        lock = false;
        break;
    }
  }
}
  

void mousePressed()
{
  if(lock == false){
    if(mouseX > xpos && mouseX < xpos + rec_width && mouseY > ypos && mouseY < ypos + rec_height) //kijkt of er op de lijn box is geklikt
    {
      toggle[0] = true;
      lock = true;
    }
    if(mouseX > xpos && mouseX < xpos + rec_width && mouseY > (ypos + spacer) && mouseY < (ypos + rec_height + spacer)) //kijkt of er op de rechthoek box is geklikt
    {
      toggle[1] = true;
      lock = true;
    }
    if(mouseX > xpos && mouseX < xpos + rec_width && mouseY > (ypos + spacer * 2) && mouseY < (ypos + rec_height + spacer * 2)) //kijkt of er op de tekst box is geklikt 
    {
      toggle[2] = true;
      lock = true;
    }
    if(mouseX > xpos && mouseX < xpos + rec_width && mouseY > (ypos + spacer * 3) && mouseY < (ypos + rec_height + spacer * 3)) //kijkt of er op de bitmap box is geklikt 
    {
      toggle[3] = true;
      lock = true;
    }
    if(mouseX > xpos && mouseX < xpos + rec_width && mouseY > (ypos + spacer * 4) && mouseY < (ypos + rec_height + spacer * 4)) //kijkt of er op de clearscherm box is geklikt
    {
      toggle[4] = true;
      lock = true;
    }
  }
}

void keyPressed() {
  if(key == '`')  //kijkt of de knop wordt ingedrukt om uit de geselcteerde commando te gaan
  {
    for(int i = 0; i < 5; i++)
    {
      toggle[i] = false;
      lock = false;
      coord_stage = 0;
    }
  }

  if(key == BACKSPACE)  //kijkt of backspace wordt ingerukt zodat de momenteel geselcteerde positie van de array of string wordt gereset
  {
    coord[coord_stage] = 0;
    if(toggle[0] == true && coord_stage == 4 ||
       toggle[1] == true && coord_stage == 4 ||
       toggle[2] == true && coord_stage == 2 ||
       toggle[4] == true && coord_stage == 0)
    {
      kleur = "";
    }
    if(toggle[2] == true && coord_stage == 3)
    {
      tekst = "";
    }
    if(toggle[2] == true && coord_stage == 4)
    {
      fontnaam = "";
    }
    if(toggle[2] == true && coord_stage == 6)
    {
      fontstijl = "";
    }
  }

  if(key == ENTER)  //kijkt of enter wordt ingerukt om de ingevoerde waarde op te slaan in de array en door te gaan naar de volgende stap
  {
    coord_stage = coord_stage + 1;
  }
  
  if((key >= '0' && key <= '9') || (key >= 'a' && key <= 'z'))  //kijkt of 0 t/m 9 en a t/m z worden ingerukt
  {
    if(toggle[0] == true && coord_stage == 4 ||
       toggle[1] == true && coord_stage == 4 ||
       toggle[2] == true && coord_stage == 2 ||
       toggle[4] == true && coord_stage == 0)
    {
      kleur += key;  //slaat de keystrokes op in de kleur array
    }else if(toggle[2] == true && coord_stage == 3)
    {
      tekst += key;  //slaat de keystrokes op in de tekst array
    }else if(toggle[2] == true && coord_stage == 4)
    {
      fontnaam += key;  //slaat de keystrokes op in de fontnaam array
    }else if(toggle[2] == true && coord_stage == 6)
    {
      fontstijl += key;  //slaat de keystrokes op in de fontstijl array
    }else{
      int digit = key - '0'; //slaat de gekozen coordinaten op in de huidige positie in de array      
      coord[coord_stage] = coord[coord_stage] * 10 + digit;
    }
  }
}
  
