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
    bool invertedCollisions;
    virtual void draw() = 0; // pure virtual → makes this class abstract
    virtual ~Shape() {}       // always provide a virtual destructor
};

class Rect: public Shape{
    bool checkCollision(float x1, float y1, int w1, int h1,
                        float x2, float y2, int w2, int h2) {
        if(!invertedCollisions){
           return !(x1 + w1 < x2 ||   // left of r2
                x1 > x2 + w2 ||   // right of r2
                y1 + h1 < y2 ||   // above r2
                y1 > y2 + h2);    // below r2 
        }else{
            return !(x1 + w1 > x2 + w2 ||   // left of r2
                x1 < x2 ||   // right of r2
                y1 + h1 > y2 + h2 ||   // above r2
                y1 < y2);    // below r2 
        }
        
    }
    public:
        // float x;
        // float y;
        // int w;
        // int h;
        uint16_t color;
        Shape* objectsToColide[10];

        Rect(float x_, float y_, int w_, int h_, uint16_t color_) {
            x = x_;
            y = y_;
            w = w_;
            h = h_;
            color = color_;
        }
        void draw(){
            display.fillRect((int)x,(int)y,w,h,color);
        }

        //collisions
        void invertCollisions(){invertedCollisions=!invertedCollisions;}
        bool checkCollisions(float x,float y,int w,int h){
            for (Shape* object : objectsToColide){
                if(object!=nullptr){
                    if (!checkCollision(x, y, w, h,
                    object->x, object->y, object->w, object->h)) return false;
                }
            }
            return true;
        }

        void move(float xInput, float yInput, float dt) {
            xInput *= dt;
            yInput *= dt;
            if (checkCollisions(x + xInput, y + yInput, w, h)) {
                x += xInput;
                y += yInput;
            }
        }
};

Rect cube1(10,10,20,20,SSD1306_INVERSE);
Rect border(0,0,128,64,SSD1306_BLACK);


void drawOnScreen(){
    border.draw();
    cube1.draw();
}

void setup() {
    Serial.begin(9600);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println("OLED not found");
            for (;;);
        }
    display.setTextColor(SSD1306_WHITE);//SSD1306_INVERSE
    display.setTextSize(1); 

    border.invertCollisions();
    cube1.objectsToColide[0]=&border;
}

unsigned long previousTime = 0;

void loop() {
    unsigned long now = millis();
    float dt = (now - previousTime) / 1000.0f; // seconds
    previousTime = now;

    display.clearDisplay();

    cube1.move(10, 0, dt);

    drawOnScreen();
    //draw on top
        //fps
        display.setCursor(0, 0);
        display.println(1.0f/dt);
    display.display();
}
