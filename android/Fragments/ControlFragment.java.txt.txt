package com.example.group7.mdp.Fragments;


import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.Fragment;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;


import com.example.group7.mdp.Arena.ArenaView;
import com.example.group7.mdp.Bluetooth.BluetoothService;
import com.example.group7.mdp.Configuration.Operation;
import com.example.group7.mdp.Configuration.Protocol;
import com.example.group7.mdp.MainActivity;
import com.example.group7.mdp.R;

import java.util.ArrayList;

import static android.content.Context.MODE_PRIVATE;

public class ControlFragment extends Fragment {
    private TextView tvStatus, tvMdfstring1, tvMdfstring2, tvIDStatus;
    private Button autoBtn, updateBtn, tiltBtn, Explore, btnF1, btnF2, btn_calibrate;
    public ArenaView arenaView;
    private SharedPreferences sharedPrefs;
    private boolean preferencesExist = false;
    private boolean autoMode = true;
    private Handler leftDelayHandler = null, rightDelayHandler = null, forwardDelayHandler = null, reverseDelayHandler = null, sendArenaHandler = null;
    private ArrayList<String> leftMessageBuffer, rightMessageBuffer;

    ImageView ctrl_top;
    ImageView ctrl_left;
    ImageView ctrl_right;
    ImageView ctrl_reverse;

//  ImageView ctrl_center;

    private static final String TAG = "ControlFragment";
    public ControlFragment() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_control, container, false);

        tvMdfstring1 = view.findViewById(R.id.tvMdfstring1);
        tvMdfstring2 = view.findViewById(R.id.tvMdfstring2);
        tvIDStatus= view.findViewById(R.id.tvIDString);
        tvStatus = view.findViewById(R.id.tvStatus);
        autoBtn = view.findViewById(R.id.btnAuto);
        updateBtn = view.findViewById(R.id.btnManual);
        updateBtn.setVisibility(View.GONE);
        tiltBtn = view.findViewById(R.id.tiltBtn);
        ctrl_top = view.findViewById(R.id.ctrl_top);
        ctrl_left = view.findViewById(R.id.ctrl_left);
        ctrl_right = view.findViewById(R.id.ctrl_right);
        ctrl_reverse = view.findViewById(R.id.ctrl_bottom);
//        ctrl_center = view.findViewById(R.id.center);
        Button sendArena = view.findViewById(R.id.SendArenaInfo);


        arenaView = ((MainActivity)getActivity()).getArenaview();
        leftMessageBuffer = new ArrayList<>();
        rightMessageBuffer = new ArrayList<>();

        sharedPrefs = getActivity().getSharedPreferences(Protocol.PROTOCOLPREF, MODE_PRIVATE);

        // NEW F1 BUTTON CONTROL
        btnF1 = view.findViewById(R.id.btnF1);
        btnF1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SharedPreferences sharedPrefs = getActivity().getSharedPreferences(Protocol.PROTOCOLPREF, MODE_PRIVATE);
                if(sharedPrefs.contains(getActivity().getResources().getString(R.string.f1))) {
                    BluetoothService.getInstance().sendText(sharedPrefs.getString(getActivity().getResources().getString(R.string.f1), ""), getActivity());
                    Operation.showToast(getActivity(), sharedPrefs.getString(getActivity().getResources().getString(R.string.f1), "") + " was sent.");
                } else {
                    BluetoothService.getInstance().sendText(Protocol.F1, getActivity());
                    Operation.showToast(getActivity(), Protocol.F1 + " was sent.");
                }
            }
        });

        // NEW F2 BUTTON CONTROL
        btnF2 = view.findViewById(R.id.btnF2);
        btnF2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SharedPreferences sharedPrefs = getActivity().getSharedPreferences(Protocol.PROTOCOLPREF, MODE_PRIVATE);
                if(sharedPrefs.contains(getActivity().getResources().getString(R.string.f2))) {
                    BluetoothService.getInstance().sendText(sharedPrefs.getString(getActivity().getResources().getString(R.string.f2), ""), getActivity());
                    Operation.showToast(getActivity(), sharedPrefs.getString(getActivity().getResources().getString(R.string.f2), "") + " was sent.");
                } else {
                    BluetoothService.getInstance().sendText(Protocol.F2, getActivity());
                    Operation.showToast(getActivity(), Protocol.F2 + " was sent.");
                }
            }
        });


        // NEW CALIBRATE
        Button btn_calibrate = view.findViewById(R.id.btn_calibrateRobot);
        btn_calibrate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                BluetoothService.getInstance().sendText(Protocol.START_CALIBRATE,getActivity());
                Operation.showToast(getActivity()," Android Controller: Calibrate sent");
            }
        } );



        //Left Rotate control button
        ctrl_left.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(leftDelayHandler == null && leftMessageBuffer.isEmpty()) {
                    setStatus(Protocol.STATUS_TURN_LEFT);
                    ((MainActivity)getActivity()).activateIdleCountDownTimer();
                    if(((MainActivity)getActivity()).getConnectedDeviceName() != null && ((MainActivity)getActivity()).getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            ((MainActivity)getActivity()).getConnectedDeviceMac() != null && ((MainActivity)getActivity()).getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        BluetoothService.getInstance().sendText(Protocol.AMD_TURN_LEFT.toString(), getActivity());
                    } else if(preferencesExist) {
                        BluetoothService.getInstance().sendText( sharedPrefs.getString(getActivity().getResources().getString(R.string.rotateLeft), ""), getActivity());
                    } else {
                        BluetoothService.getInstance().sendText( Protocol.TURN_LEFT.toString(), getActivity());
                    }
                    leftDelayHandler = new Handler();
                    Runnable r=new Runnable() {
                        public void run() {
                            if(!leftMessageBuffer.isEmpty()) {
                                setStatus(Protocol.STATUS_TURN_LEFT);
                                ((MainActivity)getActivity()).activateIdleCountDownTimer();
                                BluetoothService.getInstance().sendText(leftMessageBuffer.get(0), getActivity());
                                leftMessageBuffer.remove(0);
                                leftDelayHandler.postDelayed(this, 330);
                                arenaView.getArena().getRobot().turnLeft();
                                arenaView.invalidate();
                            } else {
                                leftDelayHandler = null;
                            }
                        }
                    };
                    leftDelayHandler.postDelayed(r, 330);
                    arenaView.getArena().getRobot().turnLeft();
                    arenaView.invalidate();
                } else {
                    if(((MainActivity)getActivity()).getConnectedDeviceName() != null && ((MainActivity)getActivity()).getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            ((MainActivity)getActivity()).getConnectedDeviceMac() != null && ((MainActivity)getActivity()).getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        leftMessageBuffer.add(Protocol.AMD_TURN_LEFT.toString());
                    } else if(preferencesExist) {
                        leftMessageBuffer.add(  sharedPrefs.getString(getActivity().getResources().getString(R.string.rotateLeft), ""));
                    } else {
                        leftMessageBuffer.add(Protocol.TURN_LEFT.toString());
                    }
                }
            }
        });

        //Right Rotate control button
        ctrl_right.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(rightDelayHandler == null && rightMessageBuffer.isEmpty()) {
                    setStatus(Protocol.STATUS_TURN_RIGHT);
                    ((MainActivity)getActivity()).activateIdleCountDownTimer();
                    if(((MainActivity)getActivity()).getConnectedDeviceName() != null && ((MainActivity)getActivity()).getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            ((MainActivity)getActivity()).getConnectedDeviceMac() != null && ((MainActivity)getActivity()).getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        BluetoothService.getInstance().sendText(Protocol.AMD_TURN_RIGHT.toString(), getActivity());
                    } else if(preferencesExist) {
                        BluetoothService.getInstance().sendText( sharedPrefs.getString(getActivity().getResources().getString(R.string.rotateRight), ""), getActivity());
                    } else {
                        BluetoothService.getInstance().sendText( Protocol.TURN_RIGHT.toString(), getActivity());
                    }
                    rightDelayHandler = new Handler();
                    Runnable r=new Runnable() {
                        public void run() {
                            if(!rightMessageBuffer.isEmpty()) {
                                setStatus(Protocol.STATUS_TURN_RIGHT);
                                ((MainActivity)getActivity()).activateIdleCountDownTimer();
                                BluetoothService.getInstance().sendText(rightMessageBuffer.get(0), getActivity());
                                rightMessageBuffer.remove(0);
                                rightDelayHandler.postDelayed(this, 330);
                                arenaView.getArena().getRobot().turnRight();
                                arenaView.invalidate();
                            } else {
                                rightDelayHandler = null;
                            }
                        }
                    };
                    rightDelayHandler.postDelayed(r, 330);
                    arenaView.getArena().getRobot().turnRight();
                    arenaView.invalidate();
                } else {
                    if(((MainActivity)getActivity()).getConnectedDeviceName() != null && ((MainActivity)getActivity()).getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            ((MainActivity)getActivity()).getConnectedDeviceMac() != null && ((MainActivity)getActivity()).getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        rightMessageBuffer.add(Protocol.AMD_TURN_RIGHT.toString());
                    } else if(preferencesExist) {
                        rightMessageBuffer.add(  sharedPrefs.getString(getActivity().getResources().getString(R.string.rotateRight), ""));
                    } else {
                        rightMessageBuffer.add( Protocol.TURN_RIGHT.toString());
                    }
                }
            }
        });

        //Move Forward control button
        ctrl_top.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(forwardDelayHandler == null) {
                    if (((MainActivity) getActivity()).getConnectedDeviceName() != null && ((MainActivity) getActivity()).getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            ((MainActivity) getActivity()).getConnectedDeviceMac() != null && ((MainActivity) getActivity()).getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        boolean check = arenaView.getArena().getRobot().moveForward(true);
                        if (check) {
                            BluetoothService.getInstance().sendText(Protocol.AMD_MOVE_FORWARD.toString(), getActivity());
                        }



                        setStatus(Protocol.STATUS_FORWARD);
                        ((MainActivity) getActivity()).activateIdleCountDownTimer();
                    } else if (arenaView.getArena().getRobot().moveForward(false)) {

                        if (preferencesExist) {
                            BluetoothService.getInstance().sendText( sharedPrefs.getString(getActivity().getResources().getString(R.string.forward), ""), getActivity());
                        } else {
                            BluetoothService.getInstance().sendText(  Protocol.MOVE_FORWARD.toString(), getActivity());
                        }
                        setStatus(Protocol.STATUS_FORWARD);
                        ((MainActivity) getActivity()).activateIdleCountDownTimer();
                    }
                    arenaView.invalidate();
                    forwardDelayHandler = new Handler();
                    Runnable r = new Runnable() {
                        public void run() {
                            forwardDelayHandler = null;
                        }
                    };
                    forwardDelayHandler.postDelayed(r, 300);
                }
            }
        });

        //Reverse control button
        ctrl_reverse.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(reverseDelayHandler == null) {
                    if (((MainActivity) getActivity()).getConnectedDeviceName() != null && ((MainActivity) getActivity()).getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            ((MainActivity) getActivity()).getConnectedDeviceMac() != null && ((MainActivity) getActivity()).getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        boolean check = arenaView.getArena().getRobot().reverse(true);
                        if (check) {
                            BluetoothService.getInstance().sendText(Protocol.AMD_REVERSE.toString(), getActivity());
                        }

                        setStatus(Protocol.STATUS_REVERSE);
                        ((MainActivity) getActivity()).activateIdleCountDownTimer();
                    } else if (arenaView.getArena().getRobot().reverse(false)) {
                        if (preferencesExist) {
                            BluetoothService.getInstance().sendText(  sharedPrefs.getString(getActivity().getResources().getString(R.string.reverse), ""), getActivity());
                        } else {
                            BluetoothService.getInstance().sendText(Protocol.REVERSE.toString(), getActivity());
                        }
                        setStatus(Protocol.STATUS_REVERSE);
                        ((MainActivity) getActivity()).activateIdleCountDownTimer();
                    }
                    arenaView.invalidate();
                    reverseDelayHandler = new Handler();
                    Runnable r = new Runnable() {
                        public void run() {
                            reverseDelayHandler = null;
                        }
                    };
                    reverseDelayHandler.postDelayed(r, 300);
                }
            }
        });

        sendArena.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try{
                    BluetoothService.getInstance().sendText("coordinate (" + Integer.toString(arenaView.getArena().getRobot().getYPos()) + "," + Integer.toString(Math.abs(arenaView.getArena().getRobot().getXPos() - 16 - 3)) + ")", getActivity());
//
                }
                catch(Exception e) {
                    e.printStackTrace();
                }



                /*if(sendArenaHandler == null) {
                    if (((MainActivity) getActivity()).getConnectedDeviceName() != null && ((MainActivity) getActivity()).getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            ((MainActivity) getActivity()).getConnectedDeviceMac() != null && ((MainActivity) getActivity()).getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        BluetoothService.getInstance().sendText(Protocol.AMD_SEND_ARENA.toString(), getActivity());
                        arenaView.getArena().getRobot().reverse(true);
                        ((MainActivity) getActivity()).activateIdleCountDownTimer();
                    } else if (arenaView.getArena().getRobot().reverse(false)) {
                        if (preferencesExist) {
                            BluetoothService.getInstance().sendText("HA:" + sharedPrefs.getString(getActivity().getResources().getString(R.string.reverse), ""), getActivity());
                        } else {
                            BluetoothService.getInstance().sendText("HA:" + Protocol.REVERSE.toString(), getActivity());
                        }
                        setStatus(Protocol.STATUS_REVERSE);
                        ((MainActivity) getActivity()).activateIdleCountDownTimer();
                    }
                    //redraw on screen
                    arenaView.invalidate();
                    reverseDelayHandler = new Handler();
                    Runnable r = new Runnable() {
                        public void run() {
                            reverseDelayHandler = null;
                        }
                    };
                    reverseDelayHandler.postDelayed(r, 300);
                }*/
            }
        });




        autoBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(autoBtn.getText().equals("Auto")) {
                    autoBtn.setText("Manual");
                    updateBtn.setVisibility(View.VISIBLE);
                    autoMode = false;
                } else {
                    autoBtn.setText("Auto");
                    updateBtn.setVisibility(View.GONE);
                    autoMode = true;
                    arenaView.invalidate();
                }
            }
        });

        updateBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                arenaView.invalidate();
            }
        });

        if(sharedPrefs.contains(getActivity().getResources().getString(R.string.forward))) {
            preferencesExist = true;
        } else {
            preferencesExist = false;
        }







        tiltBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(tiltBtn.getText().equals("On Tilt")) {
                    tiltBtn.setText("Off Tilt");
                    ((MainActivity) getActivity()).activateTiltSensing();
                } else {
                    tiltBtn.setText("On Tilt");
                    ((MainActivity) getActivity()).deactivateTiltSensing();
                }
            }
        });

        return view;
    }

    public void hideController(boolean en) {
        if(ctrl_reverse != null && ctrl_top != null && ctrl_left != null && ctrl_right != null) {
            if(en == false)
            {
                ctrl_left.setVisibility(ImageView.INVISIBLE);
                ctrl_reverse.setVisibility(ImageView.INVISIBLE);
                ctrl_right.setVisibility(ImageView.INVISIBLE);
                ctrl_top.setVisibility(ImageView.INVISIBLE);

            }
            else {
                ctrl_left.setVisibility(ImageView.VISIBLE);
                ctrl_reverse.setVisibility(ImageView.VISIBLE);
                ctrl_right.setVisibility(ImageView.VISIBLE);
                ctrl_top.setVisibility(ImageView.VISIBLE);

            }
        }
    }

    public void setStatus(String status) { tvStatus.setText("Status: " + status); }

    public void setMDF1(String mdf1) { tvMdfstring1.setText("MDF1: " + mdf1); }

    public void setMDF2(String mdf2) { tvMdfstring2.setText("MDF2: " + mdf2); }

    public void setTvStatus(String output){
        tvIDStatus.append("("+output+"),");
    }

    public void setIDString(int[] IDString){
        String output=new String();

        for(int i=0;i<3;i++){
            output+=IDString[i]+",";
        }
        tvIDStatus.append("("+output+"),");
    }

    public void resetIDString(){
        tvIDStatus.setText("ID String");
    }

    public boolean getAutoStatus() { return autoMode; }






}

