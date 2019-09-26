//Balken in Abhängigkeit des Analogwertes einblenden:
void PPM_Anzeige(){
    if(zmax != zmax_old){ 
        for(int z=(zmax+33); z<47; z++){
            tft.fillRect(10+10*z, 120, 7, 30, BLACK);
        } 
        tft.fillRect(10, 120, 7, 30, BLUE1);   //ersten Balken blau anzeigen 
        for(int z=1; z<(zmax+35); z++){
            currentcolor = BLUE1;
            if(z > 28 && z < 35)currentcolor = YELLOW;
            else if(z > 34 && z < 41)currentcolor = ORANGE;
            else if(z > 40 && z < 47)currentcolor = RED;
            if(z < 47){
                tft.fillRect(10+10*z, 120, 7, 30, currentcolor);
            }   
        }       
    }
}
