#pragma once

//This one controls the vehicle, pretty much the same as car4
class Controller {
    public:
        enum { LEFT=0, RIGHT=1, ACC=2, DEC=3, NKEYS=4 };
        
        bool key[NKEYS];
        
        void Init() {
            for (int i=0; i<NKEYS; i++) key[i]=false;
        }

        void EatKey(int keycode, int* keymap, bool pressed_or_released) {
            for (int i=0; i<NKEYS; i++){
                if (keycode==keymap[i]) key[i]=pressed_or_released;
            }
        }
        
        void Joy(int keymap, bool pressed_or_released) { 
            key[keymap]=pressed_or_released; 
        }
        
        Controller(){ Init(); } 
};