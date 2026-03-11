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
        const char* alias;
        CollisionShape* members[10] = {};

        Layer() : alias(nullptr) {}

        Layer(const char* alias_){
            memset(members, 0, sizeof(members));
            alias = alias_;
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
    // bool invertedCollisions=false;
    // virtual ~Shape() {}
    bool show=true;
    virtual void draw()=0;
};

class CollisionShape: public Shape{
    public:
        bool show=false;
        bool invertedCollisions=false;
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
            // drawable = !drawable;
            if(show){
                display.drawRect(position.x, position.y, w, h, SSD1306_WHITE);
            }
        }

        bool checkCollision(float x1, float y1, int w1, int h1,
                    float x2, float y2, int w2, int h2,
                    bool inverted) {
            if(!inverted){
                return !(x1+w1 < x2 || x1 > x2+w2 || y1+h1 < y2 || y1 > y2+h2);
            } else {
                // spaceship (x1,y1,w1,h1) must stay INSIDE border (x2,y2,w2,h2)
                return !(x1 >= x2 &&
                        y1 >= y2 &&
                        x1+w1 <= x2+w2 &&
                        y1+h1 <= y2+h2);
            }
        }
};

class MovingShape: public Shape{
    public:
        char* name;

        CollisionShape* collisions[5]={};
        CollisionShape* coliding[5]={};
        
        // virtual ~MovingShape(){}
        virtual void draw(){}
        void attachCollisions(){
            for(CollisionShape* coll : collisions){
                if(coll == nullptr) break;
                coll->position.x = position.x + coll->offset.x;
                coll->position.y = position.y + coll->offset.y;
            }
        }
        void checkCollisions(){
            memset(coliding, 0, sizeof(coliding));
            for(CollisionShape* coll : collisions){
                if(coll == nullptr) break;
                // coll->checkLayer(Layer *layer)
                for(Layer* accesLayer : coll->accesLayers){
                    if(accesLayer == nullptr) break;
                    int i=0;
                    for(CollisionShape* collAcces : accesLayer->members){
                        if(collAcces == nullptr) break;
                        if(collAcces == coll) continue; // ✓ skip self
                        bool collisionOutput = coll->checkCollision(
                                coll->position.x, coll->position.y, coll->w, coll->h,
                                collAcces->position.x, collAcces->position.y, collAcces->w, collAcces->h,
                                collAcces->invertedCollisions  // ✓ pass the flag from the OTHER shape
                        );
                        if(collisionOutput){
                            coliding[i]=collAcces;
                            // Serial.print(name);
                            // Serial.print(F(" coliding with "));
                            // Serial.println((int)collAcces);
                        }
                        i++;
                    }
                }
            }
        }
        bool move(float xInput, float yInput, float dt) {
            xInput *= dt;
            yInput *= dt;
            position.x += xInput;
            position.y += yInput;
            attachCollisions();
            checkCollisions();
            if (coliding[0]==nullptr) {
                return true;
            }else{
                position.x -= xInput;
                position.y -= yInput;
                attachCollisions();
            }
            return false;
        }
};

class RectShape: public MovingShape{
    public:
        
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
            if(show){
                display.drawRect(position.x, position.y, w, h, SSD1306_WHITE);
            }
        }
};

class Ascii: public MovingShape{
    char letters[4];
    
    public:
        template<int C>
        Ascii(char* name_, float x_, float y_, CollisionShape* (&collisions_)[C], char letters_[4]) {
            memset(collisions, 0, sizeof(collisions));   // ← add this
            memset(coliding, 0, sizeof(coliding));        // ← and this

            name = name_;
            position.x = x_;
            position.y = y_;
            Shape::w = 10;
            Shape::h = 16;
            strcpy(letters, letters_);

            for (int i = 0; i < C && i < 5; i++) {      // ← and this block
                collisions[i] = collisions_[i];
                for (Layer* layer : collisions_[i]->objectLayers) {
                    if (layer == nullptr) break;
                    layer->addCollision(collisions_[i]);
                }
            }
        }
        // void splitWith(char input[], char splitter, int howMany, char* parts[]) {
        //     static char copy[50];        // static so pointers stay valid after return
        //     strcpy(copy, input);

        //     char delim[2] = { splitter, '\0' };
        //     char* token = strtok(copy, delim);
        //     int i = 0;

        //     while (token != nullptr && i < howMany) {
        //         parts[i++] = token;
        //         token = strtok(nullptr, delim);
        //     }
        // }
        // void draw(){
        //     if(show){
        //         display.setTextColor(SSD1306_WHITE);
        //         char* parts[2];
        //         splitWith(letters, '|', 2, parts);
        //         display.setCursor(position.x,position.y);
        //         display.print(parts[0]);
        //         display.setCursor(position.x,position.y);
        //         display.print(parts[1]);
        //     }
        // }
        void draw(){
            if(show){
                display.setTextColor(SSD1306_WHITE);
                
                // char* sep = strchr(letters, '|');  // find the '|'
                // if(sep == nullptr) return;
                
                // *sep = '\0';                        // temporarily split the string
                display.setCursor(position.x, position.y);
                display.print(letters[0]);             // print first part
                display.setCursor(position.x, position.y);
                display.print(letters[2]);            // print second part
                // *sep = '|';                         // restore
            }
        }
};

class Spaceship: public Ascii{
    public:
        template<int C>
        Spaceship(char* name_, float x_, float y_, CollisionShape* (&collisions_)[C], char letters_[4])
            : Ascii(name_, x_, y_, collisions_, letters_)   // call base constructor
        {}
        void shoot(){
            Serial.println(F("PEW!"));
        }
};
// char* objectLetters[] = {
//     ">|}"//,  //starship fast
//     // "I|>",  //staarship slow
//     // "=|>",  
//     // "^|=",   //ufo
//     // "A|_"   //bullet
// };
char* spaceshipAscii=">|}";

unsigned long previousTime = 0;

   // col1R2.draw();


// Layers
    Layer phisics("phisics");
    // Layer enemy(2,"enemy",0);

    Layer* phisicsLayerPreset[1]{&phisics}; // only phisics
    Layer* noLayerPreset[1] = {nullptr}; // no layer

// Collisions
    // CollisionShape col1R1(0,0,20,20,phisicsLayerPreset,phisicsLayerPreset);
    // CollisionShape* r1Colls[1]{&col1R1};

    CollisionShape borderColl_1(0,0,128,64,phisicsLayerPreset,noLayerPreset);
    CollisionShape* borderColls[1]{&borderColl_1};

    CollisionShape spaceshipColl_1(0,0,10,14,phisicsLayerPreset,phisicsLayerPreset);
    CollisionShape* spaceshipColls[1]{&spaceshipColl_1};

// Objects
    // RectShape r1("big",5,35,20,20,r1Colls);
    RectShape border("border",0,0,128,64,borderColls);
    Spaceship spaceship("spaceship",10,20,spaceshipColls,spaceshipAscii);

//adding r1.collisionsList[j].col1ObjectLayers[i] to its layers 
void drawOnScreen(){
    // r1.draw();
    // col1R1.draw();
    // r2.draw();
    border.draw();
    spaceship.draw();
    // spaceshipColl_1.draw();
}
void setupCollisions(){
    // col1R1.drawable=true;
    border.show=true;
    // spaceshipColl_1.show=true;
    // r1.attachCollisions();
    // r2.attachCollisions();

    borderColl_1.invertedCollisions=true;
    border.attachCollisions();
    spaceship.attachCollisions();
}

void setup() {
    Wire.begin(8,9); // SDA=8, SCL=9
    randomSeed(analogRead(A0));
    Serial.begin(115200);
    // Serial.println(freeMemory());
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println(F("OLED not found"));
            for (;;);
        }
    display.setTextColor(SSD1306_WHITE);//SSD1306_INVERSE
    display.setTextSize(2);

    previousTime = millis();

    setupCollisions();
    // Serial.println(freeMemory());
}
// extern int __heap_start, *__brkval;

// int freeMemory() {
//     int v;
//     return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
// }

//In loop variables
    unsigned long lastShotTime = 0;
    const int shootCooldown = 400; // ms

void loop() {
    unsigned long now = millis();
    float dt = (now - previousTime) / 1000.0f;
    previousTime = now;

    display.clearDisplay();

    char input;
    if (Serial.available() > 0) {
        char input = 0;
        while (Serial.available() > 0) {
            input = Serial.read();  // drain all, keep only the last one
        }
        Serial.println(input);
        
        switch (input) {
            // case 's': 
            //     spaceship.move(-50, 0, dt);    break;
            // case 'w':
            //     spaceship.move(50, 0, dt);     break;
            case 'a': 
                spaceship.move(0, -50, dt);    break;
            case 'd':
                spaceship.move(0, 50, dt);     break;
            case ' ':
                if (millis() - lastShotTime > shootCooldown) {
                    spaceship.shoot();
                    lastShotTime = millis();
                }
                break;
            case 'I':
                // Serial.print(F("Memory: "));
                // Serial.print(2000-freeMemory());
                // Serial.println(F("/2000 bytes"));
                Serial.print(F("FPS: "));
                Serial.println(1.0f/dt);
                break;
        }
    }

    drawOnScreen();
    display.display();

    // delay(100);
}
