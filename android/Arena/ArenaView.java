package com.example.group7.mdp.Arena;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import com.example.group7.mdp.Configuration.Config;
import com.example.group7.mdp.Configuration.Operation;

public class ArenaView extends View {
    private Arena arena;
    private boolean inputPositionEnabled;
    private int inputPositionType;
    private int gridSize;
    public float startX,startY,endX,endY;

    public ArenaView(Context context) {
        this(context, null);
    }

    public ArenaView(Context context, AttributeSet attrs) {
        super(context, attrs);
        inputPositionEnabled = false;
    }

    public void setupArena(Arena arena) {
        this.arena = arena;
    }

    public void setInputPositionEnabled(boolean b, int t) {
        inputPositionEnabled = b;
        inputPositionType = t;
    }

    @Override
    public void onDraw(Canvas canvas) {
        gridSize = canvas.getWidth() / (Config.ARENA_LENGTH + 8); // 800 / 15 = 50
    //arena view is called arenarenderer by calling renderArena method
        //so the IDBlock14 and IDBlock15 is rendered but u sort of need set a position in arena.
        ArenaRenderer.renderArena(canvas, arena, gridSize, getContext());
    }

    public Arena getArena() {
        return arena;
    }

    @Override
    public void setOnTouchListener(OnTouchListener l) {
        super.setOnTouchListener(l);
    }

    // Ontouch event to set Robot and Waypoint X and Y Position
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (inputPositionEnabled) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                startX = (gridSize / 2)+gridSize;
                startY = gridSize / 2;


                endX = (gridSize / 2 + gridSize * Config.ARENA_LENGTH)+gridSize;
                endY = gridSize / 2 + gridSize * Config.ARENA_WIDTH;

                if (event.getX() > startX && event.getX() < endX
                        && event.getY() > startY && event.getY() < endY) {

                    int y = (int) ((event.getX() - startX) / gridSize);
                    int x = (int) ((event.getY() - startY) / gridSize);

                    try {
                        if (x >= 1 && x <= Config.ARENA_WIDTH - 2 &&
                                y >= 1 && y <= Config.ARENA_LENGTH - 2)
                        {
                            if(inputPositionType == 1)
                            {
                                System.out.println("x: "+y+",y: "+x);
                                arena.getRobot().setPosition(x, y);
                            }
                            else if (inputPositionType == 2)
                            {
                                arena.getWayPoint().setPosition(x, y);
                                //Send a message to rpi here in the format: 'wp',<x-coord>,<y-coord>
                            }
                        } else {
                            Operation.showToast(getContext(), "Invalid Position");
                        }
                    } catch (Exception e) {
                        Operation.showToast(getContext(), "Invalid Position");
                    }

                    invalidate();
                }
            }
        }


        return super.onTouchEvent(event);
    }
}