#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int KameraAufloesungx, KameraAufloesungy;
int Ax, Ay, Bx, By, Cx, Cy, Dx, Dy;
float F1x, F1y, F2x, F2y, Norm1x, Norm1y, Norm2x, Norm2y, x, y, u1x, u1y, r1x, r1y, d1, u2x, u2y, r2x, r2y, d2;

int main(){
    KameraAufloesungx = 1280;
    KameraAufloesungy = 800;
    Ax=0;Ay=0;Bx=100;By=0;Cx=0;Cy=100;Dx=110;Dy=100;
    F1x=0;F1y=0;F2x=0;F2y=0;
    int Punktx=50;
    int Punkty=0;

    //Calculation F1 and x-Coordinate
    float P1x=Dx+((Cx-Dx)*(Bx-Dx)+(Cy-Dy)*(By-Dy))*(Bx-Dx)/((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy));
    float P1y=Dy+((Cx-Dx)*(Bx-Dx)+(Cy-Dy)*(By-Dy))*(By-Dy)/((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy));
    float L1=sqrt((P1x-Cx)*(P1x-Cx)+(P1y-Cy)*(P1y-Cy));
    float sign1=((Bx-Cx)*(Dy-Ay)-(By-Cy)*(Dx-Ax)); //Mirroring correction
    float alpha=asin(((Ay-Cy)*(Bx-Dx)-(Ax-Cx)*(By-Dy))/sqrt((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy))/sqrt((Ax-Cx)*(Ax-Cx)+(Ay-Cy)*(Ay-Cy)));
    if(alpha>0.05||alpha<-0.05){
        float Abs1=L1/tan(alpha);
        F1x=P1x+((sign1>0)-(sign1<0))*(Bx-Dx)/sqrt((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy))*Abs1;
        F1y=P1y+((sign1>0)-(sign1<0))*(By-Dy)/sqrt((Bx-Dx)*(Bx-Dx)+(By-Dy)*(By-Dy))*Abs1;
        x=acos(((Ax-F1x)*(Punktx-F1x)+(Ay-F1y)*(Punkty-F1y))/sqrt((Ax-F1x)*(Ax-F1x)+(Ay-F1y)*(Ay-F1y))/sqrt((Punktx-F1x)*(Punktx-F1x)+(Punkty-F1y)*(Punkty-F1y)))/alpha;
    }else {
        u1x=(Cx-Ax)/sqrt((Cx-Ax)*(Cx-Ax)+(Cy-Ay)*(Cy-Ay));
        u1y=(Cy-Ay)/sqrt((Cx-Ax)*(Cx-Ax)+(Cy-Ay)*(Cy-Ay));
        r1x=(Bx*u1x+By*u1y-Ax*u1x-Ay*u1y)*u1x;
        r1y=(Bx*u1x+By*u1y-Ax*u1x-Ay*u1y)*u1y;
        d1=sqrt((Ax+r1x-Bx)*(Ax+r1x-Bx)+(Ay+r1y-By)*(Ay+r1y-By));
        Norm1x=((sign1>0)-(sign1<0))*u1y;
        Norm1y=-((sign1>0)-(sign1<0))*u1x;
        x=((Punktx-Ax)*Norm1x+(Punkty-Ay)*Norm1y)/d1;
    }

    //Calculation F2 and y-Coordinate
    float P2x=Cx+((Ax-Cx)*(Dx-Cx)+(Ay-Cy)*(Dy-Cy))*(Dx-Cx)/((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy));
    float P2y=Cy+((Ax-Cx)*(Dx-Cx)+(Ay-Cy)*(Dy-Cy))*(Dy-Cy)/((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy));
    float L2=sqrt((P2x-Ax)*(P2x-Ax)+(P2y-Ay)*(P2y-Ay));
    float sign2=((Dx-Ax)*(Cy-By)-(Dy-Ay)*(Cx-Bx)); //Mirroring correction
    float beta=asin(((By-Ay)*(Dx-Cx)-(Bx-Ax)*(Dy-Cy))/sqrt((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy))/sqrt((Bx-Ax)*(Bx-Ax)+(By-Ay)*(By-Ay)));
    if(beta>0.05||beta<-0.05){
        float Abs2=L2/tan(beta);
        F2x=P2x+((sign2>0)-(sign2<0))*(Dx-Cx)/sqrt((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy))*Abs2;
        F2y=P2y+((sign2>0)-(sign2<0))*(Dy-Cy)/sqrt((Dx-Cx)*(Dx-Cx)+(Dy-Cy)*(Dy-Cy))*Abs2;
        y=acos(((Ax-F2x)*(Punktx-F2x)+(Ay-F2y)*(Punkty-F2y))/sqrt((Ax-F2x)*(Ax-F2x)+(Ay-F2y)*(Ay-F2y))/sqrt((Punktx-F2x)*(Punktx-F2x)+(Punkty-F2y)*(Punkty-F2y)))/beta;
    }else{
        u2x=(Bx-Ax)/sqrt((Bx-Ax)*(Bx-Ax)+(By-Ay)*(By-Ay));
        u2y=(By-Ay)/sqrt((Bx-Ax)*(Bx-Ax)+(By-Ay)*(By-Ay));
        r2x=(Cx*u2x+Cy*u2y-Ax*u2x-Ay*u2y)*u2x;
        r2y=(Cx*u2x+Cy*u2y-Ax*u2x-Ay*u2y)*u2y;
        d2=sqrt((Ax+r2x-Cx)*(Ax+r2x-Cx)+(Ay+r2y-Cy)*(Ay+r2y-Cy));
        Norm2x=-((sign2>0)-(sign2<0))*u2y;
        Norm2y=((sign2>0)-(sign2<0))*u2x;
        y=((Punktx-Ax)*Norm2x+(Punkty-Ay)*Norm2y)/d2;
    }

    printf("%f,%f,%f,%f,%f,%f,%f,%f\n",Norm1x,Norm1y,Norm2x,Norm2y,x,y,d1,d2);
    return 0;
}
