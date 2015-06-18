#ifndef __PROJECT_H
#define __PROJECT_H

#include <gtk/gtk.h>

/*
    Techniki Programowania 4
    Zadanie 9a

    Zad 9

    Zaimplementować program zawierający GUI w środowisku WinAPI,
    który wczytuje, przetwarza i wizualizuje dane aktualnego położenia
    kątowego ( roll, pitch, yaw ) robota mobilnego. Program ma wyświetlać( w GUI )
    aktualną linię horyzontu w rzucie z góry, z przodu i z boku.
    W GUI należy dodać przyciski odpowiedzialne za wyświetlanie wymaganych elementów.

    outputRotateB01.log

*/

namespace project
{
    void init( GtkBuilder* );
}

#endif
