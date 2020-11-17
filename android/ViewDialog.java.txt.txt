package com.example.group7.mdp;

import android.app.Dialog;
import android.content.Context;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageView;

import com.example.group7.mdp.Bluetooth.BluetoothService;


public class ViewDialog {

    private MainActivity mainActivity;
    private Context context;

    public ViewDialog(Context context) {
        this.context = context;
        this.mainActivity = (MainActivity) context;
    }

    public final void showDialog(String title, String msg) {
        final Dialog dialog = new Dialog(context);

        dialog.setTitle(title);
        dialog.setCancelable(true);
        dialog.setContentView(R.layout.dialog_main);

        ImageView imgRobot = (ImageView) dialog.findViewById(R.id.imgRobot);
        imgRobot.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mainActivity.setRobotPosition(true);
                mainActivity.inputWayPointPosition(1);
                dialog.dismiss();
            }
        });

        ImageView imgWaypoint = (ImageView) dialog.findViewById(R.id.imgWaypoint);
        imgWaypoint.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mainActivity.setRobotPosition(false);
                mainActivity.inputWayPointPosition(2);
                dialog.dismiss();
            }
        });

        // Submit Button
        Button btnSubmit = (Button) dialog.findViewById(R.id.btnSubmit);
        btnSubmit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //robot position x, robot position y, waypoint x, waypoint y
                int r_x, r_y, w_x, w_y;

                // Robot initial value for X and Y
                EditText spInitialX = (EditText) dialog.findViewById(R.id.spInitialX);
                EditText spInitialY = (EditText) dialog.findViewById(R.id.spInitialY);

                // Waypoint initial value for X and Y
                EditText spwpInitialX = (EditText) dialog.findViewById(R.id.spwpInitialX);
                EditText spwpInitialY = (EditText) dialog.findViewById(R.id.spwpInitialY);


                // Get robot x and y pos
                r_x = mainActivity.mod(Integer.parseInt(spInitialX.getText().toString()), 14);
                r_y = mainActivity.mod(19 - Integer.parseInt(spInitialY.getText().toString()), 19);

                // Get waypoint x and y pos
                int old_waypoint_x = mainActivity.getArenaview().getArena().getWayPoint().getXPos();
                int old_waypoint_y = mainActivity.getArenaview().getArena().getWayPoint().getYPos();
                w_x = mainActivity.mod(Integer.parseInt(spwpInitialX.getText().toString()), 14);
                w_y = mainActivity.mod(19 - Integer.parseInt(spwpInitialY.getText().toString()), 19);

                Integer.parseInt(spwpInitialY.getText().toString());


                mainActivity.getArenaview().getArena().getWayPoint().setPosition(w_y, w_x);


                mainActivity.getArenaview().getArena().getRobot().setPosition(r_y, r_x);
             /*   if ((mainActivity.mod((old_waypoint_y - 14), 14) == w_x) && (mainActivity.mod((19 - old_waypoint_x), 19) == 19 - w_y)) {
                    BluetoothService.getInstance().sendText("No change in Waypoint coordinates", mainActivity);
                    dialog.dismiss();
                }
                else{
                BluetoothService.getInstance().sendText("Waypoint position changed from X: " +
                        String.valueOf(mainActivity.mod((old_waypoint_y - 14), 14)) + ", Y: " + String.valueOf(mainActivity.mod((19 - old_waypoint_x), 19)) + " to X: " +
                        w_x + ", Y: " + Integer.toString(19 - w_y) + "\n", mainActivity);
                mainActivity.getArenaview().invalidate();

                //Close dialog
                dialog.dismiss();
                }*/
            }
        });

        // Close Button
        Button btnClose = (Button) dialog.findViewById(R.id.btnClose);
        btnClose.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.dismiss();
            }
        });

        dialog.show();
    }
}