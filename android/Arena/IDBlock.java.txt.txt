package com.example.group7.mdp.Arena;

import android.util.Log;

import static android.content.ContentValues.TAG;

public class IDBlock {

    private int xPos, yPos;
    private boolean render=false;


    public IDBlock(int xPos, int yPos) {
        this.xPos = xPos;
        this.yPos = yPos;
    }

    public void setRender(){
        this.render=true;
        Log.d(TAG, "setRender: render has been updated");
    }

    public void resetRender(){
        if(this.render==true){
            this.render=false;
            Log.d(TAG, "setRender: render has been reset");
        }
    }

    public boolean getRender(){
        return render;
    }

    public int getXPos() {
        return xPos;
    }

    public int getYPos() {
        return yPos;
    }

    public void setPosition(int xPos, int yPos) {
        this.xPos = Math.abs(yPos-21);
        this.yPos = xPos;
    }
}



