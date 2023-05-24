#include <iostream>
#include "stdio.h"
#include "string.h"
#include <sstream>
#include "Window.h"

int main(){
    Window myWindow = Window();

    IMG beer("ressources/Beer.png");
    IMG rat("ressources/Rat.png");
    IMG heart("ressources/Heart.png");
    IMG banane("ressources/Banane.png");

    float rotationFactor = 0;
    float zoomFactor = 3;

    float maxZoom = 10;
    float minZoom = 2;

    bool zoom = true;

    do {
        myWindow.DrawWholeWindow(BLACK);

        for (int i = -1; i < WINDOW_WIDTH / rat.width / zoomFactor + 1; i++)
        {
            for (int j = -1; j < WINDOW_HEIGHT / rat.height / zoomFactor + 1; j++)
            {
                myWindow.DrawRotatedIMG(myWindow.ZoomedIMG(rat, zoomFactor),
                rat.width * i * zoomFactor, rat.height * j * zoomFactor, rotationFactor, PivotType::TopLeft);
            }
        }
        
        // myWindow.DrawIMG(beer, 100, 100);
        // myWindow.DrawRotatedIMG(rat, 200, 200, 1.5f);
        // myWindow.DrawZoomedIMG(heart, 300, 300, 7.2f);

        // myWindow.DrawRotatedIMG(myWindow.ZoomedIMG(rat, 7.5f), 400,200, 1.5f+ x);
        // myWindow.DrawIMG(myWindow.ZoomedIMG(rat, 7.5f), 100, 100);

        // myWindow.DrawRectShape(400,200,0+x,20,RED);
        // myWindow.DrawFullRect(100,400,500,300+x, GREEN);
        // myWindow.DrawRectShape(WINDOW_WIDTH,WINDOW_HEIGHT,0,0,BLACK);
        
        if (zoomFactor >= maxZoom)
            zoom = false;
        else if (zoomFactor <= minZoom)
            zoom = true;
        
        if (zoom)
            zoomFactor +=0.5f;
        else
            zoomFactor -=0.5f;

        rotationFactor -= 0.1f;

        myWindow.SetState();

        if (myWindow.GetState() < 0) {
            myWindow.window = NULL;
            break;
        }
    } while(mfb_wait_sync(myWindow.window));
}