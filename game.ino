#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// bool DEBUG=false;
class CollisionShape;
class Vector2;
class Layer;

class Vector2{
    public:
        float x;
        float y;
        Vector2() : x(0), y(0) {}
        Vector2(float x_, float y_){
            x=x_;
            y=y_;
        }
};

class Layer{
    public:
        int number;
        const char* alias;
        int numberOfMembers;
        CollisionShape* members[10] = {};

        Layer() : number(0), alias(nullptr), numberOfMembers(0) {}

        Layer(int number_, const char* alias_, int numberOfMembers_){
            memset(members, 0, sizeof(members));
            number = number_;
            alias = alias_;
            numberOfMembers = numberOfMembers_;
        }

        void addCollision(CollisionShape* collision){
            int i=0;
            for(CollisionShape* member : members){
                if(member==nullptr){
                    members[i]=collision;
                    return;
                }
                i++;
            }
        }
};

class Shape {
public:
    Vector2 position;
    int w, h;
    uint16_t color;
    // bool invertedCollisions=false;
    virtual ~Shape() {}
    bool drawable;
    virtual void draw()=0;
};

class CollisionShape: public Shape{
    public:
        bool drawable=false;
        Layer* objectLayers[5];
        Layer* accesLayers[5];

        Vector2 offset;

        template<int O, int A>
        CollisionShape(Layer* (&objectLayers_)[O], Layer* (&accesLayers_)[A]){
            memset(objectLayers, 0, sizeof(objectLayers));
            memset(accesLayers, 0, sizeof(accesLayers));

            for (int i = 0; i < O && i < 5; i++) {
                objectLayers[i] = objectLayers_[i];
            }
            for (int i = 0; i < A && i < 5; i++) {
                accesLayers[i] = accesLayers_[i];
            }
        }
        template<int O, int A>
        CollisionShape(float oX_, float oY_, int w_, int h_, Layer* (&objectLayers_)[O], Layer* (&accesLayers_)[A]){
            memset(objectLayers, 0, sizeof(objectLayers));
            memset(accesLayers, 0, sizeof(accesLayers));

            offset.x = oX_;
            offset.y = oY_;
            w = w_;
            h = h_;
            for (int i = 0; i < O && i < 5; i++) {
                objectLayers[i] = objectLayers_[i];
            }
            for (int i = 0; i < A && i < 5; i++) {
                accesLayers[i] = accesLayers_[i];
            }
        }
        void draw(){
            drawable = !drawable;
            if(drawable){
                display.drawRect(position.x, position.y, w, h, SSD1306_WHITE);
            }
        }

        bool checkCollision(float x1, float y1, int w1, int h1,
                float x2, float y2, int w2, int h2
                ) {
            if(true){
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
};

class RectShape: public Shape{
    public:
        bool drawable=true;
        char* name;

        CollisionShape* collisions[5]={};
        CollisionShape* coliding[5]={};
        template<int C>
        RectShape(char* name_, float x_, float y_, int w_, int h_, CollisionShape* (&collisions_)[C]){
            memset(collisions, 0, sizeof(collisions));
            memset(coliding, 0, sizeof(coliding));

            name=name_;
            position.x = x_;
            position.y = y_;
            w = w_;
            h = h_;
            for (int i = 0; i < C && i < 5; i++) {
                collisions[i] = collisions_[i];
                CollisionShape* collision = collisions_[i];
                for(Layer* layer : collision->objectLayers){
                    if(layer == nullptr) break; // ✓
                    layer->addCollision(collision);
                }
            }
        }
        void draw(){
            if(drawable){
                display.drawRect(position.x, position.y, w, h, SSD1306_WHITE);
            }
        }
        void attachCollisions(){
            for(CollisionShape* coll : collisions){
                if(coll == nullptr) break;
                coll->position.x = position.x + coll->offset.x;
                coll->position.y = position.y + coll->offset.y;
            }
        }
        void checkCollisions(){
            for(CollisionShape* coll : collisions){
                if(coll == nullptr) break;
                // coll->checkLayer(Layer *layer)
                for(Layer* accesLayer : coll->accesLayers){
                    if(accesLayer == nullptr) break;
                    int i=0;
                    for(CollisionShape* collAcces : accesLayer->members){
                        if(collAcces == nullptr) break;
                        if(collAcces == coll) continue; // ✓ skip self
                        bool collisionOutput = coll->checkCollision(coll->position.x, coll->position.y, coll->w, coll->h,
                                collAcces->position.x, collAcces->position.y, collAcces->w, collAcces->h);
                        if(collisionOutput){
                            coliding[i]=collAcces;
                            Serial.print(name);
                            Serial.print(F(" coliding with "));
                            Serial.println((int)collAcces);
                        }
                        i++;
                    }
                }
            }
        }
        bool move(float xInput, float yInput, float dt) {
            xInput *= dt;
            yInput *= dt;
            checkCollisions();
            if (coliding) {
                position.x += xInput;
                position.y += yInput;
                attachCollisions();
                return true;
            }
            return false;
        }
};

// class Asci: public Shape{
//     char letters[4];
    
//     public:
//         Asci(float x_, float y_, char letters_[4], int layer_, uint16_t color_) {
//             x = x_;
//             y = y_;
//             layer = layer_;
//             strcpy(letters, letters_);
//             color = color_;
//             Shape::w = 10; // 2 chars × 12px at textSize 2
//             Shape::h = 16; // 16px tall at textSize 2
//         }
//         void splitWith(char input[], char splitter, int howMany, char* parts[]) {
//             static char copy[50];        // static so pointers stay valid after return
//             strcpy(copy, input);

//             char delim[2] = { splitter, '\0' };
//             char* token = strtok(copy, delim);
//             int i = 0;

//             while (token != nullptr && i < howMany) {
//                 parts[i++] = token;
//                 token = strtok(nullptr, delim);
//             }
//         }
//         void draw(){
//             display.setTextColor(color);
//             char* parts[2];
//             splitWith(letters, '|', 2, parts);
//             display.setCursor(x,y);
//             display.print(parts[0]);
//             display.setCursor(x,y);
//             display.print(parts[1]);
//         }
// };

char* objectLetters[] = {
    ">|}",  //starship fast
    "I|>",  //staarship slow
    "=|>",  
    "^|=",   //ufo
    "A|_"   //bullet
};

unsigned long previousTime = 0;

void drawOnScreen(){
}

// Layers
    Layer phisics(1,"phisics",0);
    // Layer enemy(2,"enemy",0);

    Layer* phisicsLayerPreset[1]{&phisics}; // only phisics
    Layer* noLayerPreset[1]; // no layer

CollisionShape col1R1(0,0,20,20,phisicsLayerPreset,phisicsLayerPreset);
CollisionShape* r1Colls[1]{&col1R1};

CollisionShape col1R2(0,0,13,18,phisicsLayerPreset,noLayerPreset);
CollisionShape* r2Colls[1]{&col1R2};

RectShape r1("big",5,5,20,20,r1Colls);
RectShape r2("small",8,15,13,18,r2Colls);
//adding r1.collisionsList[j].col1ObjectLayers[i] to its layers 

void setupCollisions(){
    // col1R1.drawable=true;
    // col1R2.drawable=true;
    r1.attachCollisions();
    r2.attachCollisions();
}

void setup() {
    Wire.begin();
    randomSeed(analogRead(A0));
    Serial.begin(115200);
    Serial.println(freeMemory());
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println(F("OLED not found"));
            for (;;);
        }
    display.setTextColor(SSD1306_WHITE);//SSD1306_INVERSE
    display.setTextSize(2);

    previousTime = millis();

    setupCollisions();
}
extern int __heap_start, *__brkval;

int freeMemory() {
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}


unsigned long debugTimer = 0;

void loop() {
    Serial.print(F("Memory: "));
    Serial.print(2000-freeMemory());
    Serial.println(F("/2000 bytes"));
    unsigned long now = millis();
    float dt = (now - previousTime) / 1000.0f;
    previousTime = now;

    display.clearDisplay();

    r1.draw();
    // col1R1.draw();
    r1.checkCollisions();

    r2.draw();
    // col1R2.draw();
    r2.checkCollisions();

    drawOnScreen();
    display.display();

    delay(100);
}
