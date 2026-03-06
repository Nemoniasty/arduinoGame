#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

class Shape {
public:
    float x;
    float y;
    int w;
    int h;
    int layer;
    uint16_t color;
    bool invertedCollisions=false;
    Shape* objectsToColide[20];

    bool checkCollision(float x1, float y1, int w1, int h1,
                        float x2, float y2, int w2, int h2,
                        bool invertedCollisions) {
        if(!invertedCollisions){
           return !(x1 + w1 < x2 ||   // left of r2
                x1 > x2 + w2 ||   // right of r2
                y1 + h1 < y2 ||   // above r2
                y1 > y2 + h2);    // below r2 
        }else{
            return (x1 >= x2 &&
                y1 >= y2 &&
                x1 + w1 <= x2 + w2 &&
                y1 + h1 <= y2 + h2);
        }
    }
    template<int N>
    void addColliders(Shape* (&obj)[N]) {
        for (int i = 0; i < N && i < 20; i++) {
            objectsToColide[i] = obj[i];
        }
    }
    void invertCollisions(){invertedCollisions=!invertedCollisions;}
    bool checkCollisions(float x,float y,int w,int h, int layerInput){
        // Serial.println("Checking collisions for: ");
        for (Shape* object : objectsToColide){
            if(object!=nullptr){
                // Serial.print("{");
                // Serial.print(object->w);
                // Serial.print(",");
                // Serial.print(object->h);
                // Serial.print("} - ");
                if (!checkCollision(x, y, w, h,
                object->x, object->y, object->w, object->h, object->invertedCollisions)&&object->layer==layerInput){
                    // Serial.println("false");
                    return false;
                    break;
                } 
            }
        }
        // Serial.println("true");
        return true;
    }

    bool canMove(float xInput, float yInput, float dt) {
        xInput *= dt;
        yInput *= dt;
        if (checkCollisions(x + xInput, y + yInput, w, h,layer)) {
            return true;
        }
        return false;
    }

    bool move(float xInput, float yInput, float dt) {
        xInput *= dt;
        yInput *= dt;
        if (checkCollisions(x + xInput, y + yInput, w, h,layer)) {
            x += xInput;
            y += yInput;
            return true;
        }
        return false;
    }
    
    // virtual void draw() = 0; // pure virtual → makes this class abstract
    // virtual void draw(int mode) = 0; // pure virtual → makes this class abstract
    virtual ~Shape() {
        for(int i=0;i<20;i++) objectsToColide[i] = 0;
    }       // always provide a virtual destructor
};


class Rect: public Shape{
    public:
        // Shape* objectsToColide[3] = {nullptr, nullptr, nullptr};

        Rect(float x_, float y_, int w_, int h_,int layer_, uint16_t color_) {
            x = x_;
            y = y_;
            w = w_;
            h = h_;
            layer=layer_;
            color = color_;
        }
        void draw(int mode){
            if(mode==1)
            display.fillRect((int)x,(int)y,w,h,color);
            else
            display.drawRect((int)x,(int)y,w,h,color);
        }
};

class Asci: public Shape{
    char letters[4];
    
    public:
        Asci(float x_, float y_, char letters_[4], int layer_, uint16_t color_) {
            x = x_;
            y = y_;
            layer = layer_;
            strcpy(letters, letters_);
            color = color_;
            Shape::w = 10; // 2 chars × 12px at textSize 2
            Shape::h = 16; // 16px tall at textSize 2
        }
        void splitWith(char input[], char splitter, int howMany, char* parts[]) {
            static char copy[50];        // static so pointers stay valid after return
            strcpy(copy, input);

            char delim[2] = { splitter, '\0' };
            char* token = strtok(copy, delim);
            int i = 0;

            while (token != nullptr && i < howMany) {
                parts[i++] = token;
                token = strtok(nullptr, delim);
            }
        }
        void draw(){
            display.setTextColor(color);
            char* parts[2];
            splitWith(letters, '|', 2, parts);
            display.setCursor(x,y);
            display.print(parts[0]);
            display.setCursor(x,y);
            display.print(parts[1]);
        }
};

char* objectLetters[] = {
    ">|}",  //starship fast
    "I|>",  //staarship slow
    "=|>",  
    "^|=",   //ufo
    "A|_"   //bullet
};

unsigned long previousTime = 0;
// Rect cube1(10,10,5,5,0,SSD1306_INVERSE);
Rect border(0,0,128,64,0,SSD1306_INVERSE);
Asci ship(10,24,objectLetters[0],0,SSD1306_WHITE);

void drawOnScreen(){
    // border.draw(0);
    // cube1.draw(1);
    ship.draw();
}
void setupColliders(){
    border.invertCollisions();
    Shape* shapes[1]{&border};
    // cube1.addColliders(shapes);
    // ship.addColliders(shapes);
    ship.addColliders(shapes);
}

void setup() {
    Wire.begin();
    // Wire.setClock(100000);
    randomSeed(analogRead(A0));
    Serial.begin(115200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println("OLED not found");
            for (;;);
        }
    display.setTextColor(SSD1306_WHITE);//SSD1306_INVERSE
    display.setTextSize(2);

    previousTime = millis();

    setupColliders();
}
extern int __heap_start, *__brkval;

int freeMemory() {
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}


// int dirX=1;
// int dirY=1;
// int xSpeed = random(2) ? random(10,30) : random(-30,-10);
// int ySpeed = random(2) ? random(10,30) : random(-30,-10);

unsigned long debugTimer = 0;
// bool moveLeft = false;
// bool moveRight = false;

void loop() {
    unsigned long now = millis();
    float dt = (now - previousTime) / 1000.0f;
    previousTime = now;

    display.clearDisplay();

    // display.setRotation(1);

    // if(!cube1.canMove(xSpeed*dirX, 0, dt)) dirX=-dirX;
    // if(!cube1.canMove(0, ySpeed*dirY, dt)) dirY=-dirY;

    // cube1.move(xSpeed*dirX, ySpeed*dirY, dt);

    // if(now - debugTimer > 200){
    //     Serial.println((int)freeMemory());
    //     debugTimer = now;
    // }

    char input;
    if (Serial.available() > 0) {
        input = Serial.read();
        Serial.println(input);
        
        switch (input) {
            case 'a': 
                ship.move(0, -60, dt);
                break;
            case 'd':
                ship.move(0, 60, dt);
                break;
        }
        

    }

    drawOnScreen();
    display.display();
}
