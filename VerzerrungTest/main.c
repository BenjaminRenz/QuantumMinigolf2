#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int KameraAufloesungx, KameraAufloesungy;
int Ax, Ay, Bx, By, Cx, Cy, Dx, Dy;
float F1x, F1y, F2x, F2y, Norm1x, Norm1y, Norm2x, Norm2y, x, y;

int main(){
    KameraAufloesungx = 1280;
    KameraAufloesungy = 800;
    Ax=0;Ay=0;Bx=100;By=0;Cx=0;Cy=100;Dx=100;Dy=100;
    F1x=0;F1y=0;F2x=0;F2y=0;
    int Punktx=50;
    int Punkty=50;
    //Calculation F1
    float P1x=Cx+((Ax-Cx)*(Dx-Cx)+(Ay-Cy)*(Dy-Cy))*(Dx-Cx)/((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy));
    float P1y=Cy+((Ax-Cx)*(Dx-Cx)+(Ay-Cy)*(Dy-Cy))*(Dy-Cy)/((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy));
    float L1=sqrt((P1x-Ax)*(P1x-Ax)+(P1y-Ay)*(P1y-Ay));
    float sign1=((Dx-Ax)*(Cy-By)-(Dy-Ay)*(Cx-Bx)); //Mirroring correction
    float alpha=asin(((By-Ay)*(Dx-Cx)-(Bx-Ax)*(Dy-Cy))/sqrt((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy))/sqrt((Bx-Ax)*(Bx-Ax)+(By-Ay)*(By-Ay)));
    if(alpha>0.05||alpha<-0.05){
        float Abs1=L1/tan(alpha);
        F1x=P1x+((sign1>0)-(sign1<0))*(Dx-Cx)/sqrt((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy))*Abs1;
        F1y=P1y+((sign1>0)-(sign1<0))*(Dy-Cy)/sqrt((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy))*Abs1;
    }else{
        Norm1x=-((sign1>0)-(sign1<0))*(By-Ay)/sqrt((Bx-Ax)*(Bx-Ax)+(By-Ay)*(By-Ay));
        Norm1y=((sign1>0)-(sign1<0))*(Bx-Ax)/sqrt((Bx-Ax)*(Bx-Ax)+(By-Ay)*(By-Ay));
        x=((Punktx-Ax)*Norm1x+(Punkty-Ay)*Norm1y);
    }

    //Calculation F2
    float P2x=Dx+((Cx-Dx)*(Bx-Dx)+(Cy-Dy)*(By-Dy))*(Bx-Dx)/((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy));
    float P2y=Dy+((Cx-Dx)*(Bx-Dx)+(Cy-Dy)*(By-Dy))*(By-Dy)/((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy));
    float L2=sqrt((P2x-Cx)*(P2x-Cx)+(P2y-Cy)*(P2y-Cy));
    float sign2=((Bx-Cx)*(Dy-Ay)-(By-Cy)*(Dx-Ax)); //Mirroring correction
    float beta=asin(((Ay-Cy)*(Bx-Dx)-(Ax-Cx)*(By-Dy))/sqrt((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy))/sqrt((Ax-Cx)*(Ax-Cx)+(Ay-Cy)*(Ay-Cy)));
    if(beta>0.05||beta<-0.05){
        float Abs2=L2/tan(beta);
        F2x=P2x+((sign2>0)-(sign2<0))*(Bx-Dx)/sqrt((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy))*Abs2;
        F2y=P2y+((sign2>0)-(sign2<0))*(By-Dy)/sqrt((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy))*Abs2;
    }else {
        Norm2x=((sign2>0)-(sign2<0))*(Cy-Ay)/sqrt((Cx-Ax)*(Cx-Ax)+(Cy-Ay)*(Cy-Ay));
        Norm2y=-((sign2>0)-(sign2<0))*(Cx-Ax)/sqrt((Cx-Ax)*(Cx-Ax)+(Cy-Ay)*(Cy-Ay));
        y=((Punktx-Ax)*Norm2x+(Punkty-Ay)*Norm2y);
    }

    printf("%f,%f,%f,%f,%f,%f\n",Norm1x,Norm1y,Norm2x,Norm2y,x,y);
    return 0;
}
