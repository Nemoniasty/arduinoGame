#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64e
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

class Shape {
public:
    float x;
    float y;
    int w;
    int h;
    int layer;
    bool invertedCollisions=false;
    virtual void draw(String mode) = 0; // pure virtual → makes this class abstract
    virtual bool checkCollision(float x1, float y1, int w1, int h1,
                        float x2, float y2, int w2, int h2, bool invertedCollisions);
    virtual ~Shape() {}       // always provide a virtual destructor
};

class Rect: public Shape{
    bool checkCollision(float x1, float y1, int w1, int h1,
                        float x2, float y2, int w2, int h2,
                        bool invertedCollisions) {
        if(!invertedCollisions){
           return !(x1 + w1 < x2 ||   // left of r2
                x1 > x2 + w2 ||   // right of r2
                y1 + h1 < y2 ||   // above r2
                y1 > y2 + h2);    // below r2 
        }else{
            // return !(x1 + w1 > x2 + w2 ||   // left of r2
            //     x1 < x2 ||   // right of r2
            //     y1 + h1 > y2 + h2 ||   // above r2
            //     y1 < y2);    // below r2 
            return (x1 >= x2 &&
                y1 >= y2 &&
                x1 + w1 <= x2 + w2 &&
                y1 + h1 <= y2 + h2);
        }
        
    }
    public:
        // float x;
        // float y;
        // int w;
        // int h;
        uint16_t color;
        Shape* objectsToColide[3] = {nullptr, nullptr, nullptr};

        Rect(float x_, float y_, int w_, int h_,int layer_, uint16_t color_) {
            x = x_;
            y = y_;
            w = w_;
            h = h_;
            layer=layer_;
            color = color_;
        }
        void draw(String mode){
            if(mode=="full")
            display.fillRect((int)x,(int)y,w,h,color);
            else
            display.drawRect((int)x,(int)y,w,h,color);
        }

        //collisions
        void addCollider(Shape* obj[]) {
            for (int i = 0; i < (   sizeof(objectsToColide) / sizeof(objectsToColide[0])    ); i++) {
                if (objectsToColide[i] == nullptr) {
                    objectsToColide[i] = obj[i];
                    return;
                }
            }
        }
        void invertCollisions(){invertedCollisions=!invertedCollisions;}
        bool checkCollisions(float x,float y,int w,int h, int layerInput[]){
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
            if (checkCollisions(x + xInput, y + yInput, w, h,0)) {
                return true;
            }
            return false;
        }

        bool move(float xInput, float yInput, float dt) {
            xInput *= dt;
            yInput *= dt;
            if (checkCollisions(x + xInput, y + yInput, w, h,0)) {
                x += xInput;
                y += yInput;
                return true;
            }
            return false;
        }
};

Rect cube1(10,10,5,5,0,SSD1306_INVERSE);
Rect border(5,5,40,40,0,SSD1306_INVERSE);

void drawOnScreen(){
    border.draw("hollow");
    cube1.draw("full");
    // display.drawFastVLine(128,0,64,SSD1306_INVERSE);
}
void setupColliders(){
    border.invertCollisions();
    Shape* shapes[1]{&border};
    cube1.addCollider(shapes);
}

void setup() {
    Wire.begin();
    Wire.setClock(400000);
    randomSeed(analogRead(A0));
    Serial.begin(9600);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println("OLED not found");
            for (;;);
        }
    display.setTextColor(SSD1306_INVERSE);//SSD1306_INVERSE
    display.setTextSize(1);

    setupColliders();
}

unsigned long previousTime = 0;
int dirX=1;
int dirY=1;
int xSpeed = random(2) ? random(10,30) : random(-30,-10);
int ySpeed = random(2) ? random(10,30) : random(-30,-10);

void loop() {
    unsigned long now = millis();
    float dt = (now - previousTime) / 1000.0f; // seconds
    previousTime = now;

    display.clearDisplay();

    // Serial.println(xSpeed+" "+ySpeed);
    if(!cube1.canMove(xSpeed*dirX, 0, dt)) dirX=-dirX;

    if(!cube1.canMove(0, ySpeed*dirY, dt)) dirY=-dirY;
    
    cube1.move(xSpeed*dirX, ySpeed*dirY, dt);


    drawOnScreen();
        Serial.println(1.0f/dt);
    display.display();
}
