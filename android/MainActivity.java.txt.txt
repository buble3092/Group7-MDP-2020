package com.example.group7.mdp;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.PorterDuff;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.support.annotation.NonNull;
import android.support.design.widget.NavigationView;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.ContextCompat;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import android.hardware.SensorEventListener;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorManager;

import com.example.group7.mdp.Arena.Arena;
import com.example.group7.mdp.Arena.ArenaView;
import com.example.group7.mdp.Arena.Robot;
import com.example.group7.mdp.Bluetooth.BluetoothDialog;
import com.example.group7.mdp.Bluetooth.BluetoothService;
import com.example.group7.mdp.Configuration.Config;
import com.example.group7.mdp.Configuration.Operation;
import com.example.group7.mdp.Configuration.Protocol;
import com.example.group7.mdp.Fragments.ControlFragment;
import com.example.group7.mdp.Fragments.DebugFragment;

import java.io.File;
import java.io.FileWriter;
import java.nio.charset.Charset;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Vector;
import java.util.ArrayList; // import the ArrayList class

import static android.hardware.SensorManager.SENSOR_DELAY_UI;

public class MainActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener, SensorEventListener {

    private static final String TAG ="FINAL_CODE";
    private RelativeLayout btnExplore, btnFastestPath;
    private Button SendArenabutton;
    private NavigationView navigationView;
    private BluetoothAdapter mBluetoothAdapter = null;
    private BluetoothDialog btDialog = null;
    private BluetoothService btService = null;
    private ViewDialog vdDialog = null;
    private final static int REQUEST_ENABLE_BT = 777;
    private final static int REQUEST_ALLOW_COARSE_LOCATION = 888;
    private ImageView btIcon;
    private TextView btStatus, exploreTV, fastestTV, exploreTimeTV, fastestTimeTV, oneMinuteTimerTV;
    private String mConnectedDeviceName = null;
    private String mConnectedDeviceMac = null;
    Handler handlerAutoUpdate = new Handler();
    private DebugFragment debugFragment = null;


    private ControlFragment ctrlFragment = null;
    private boolean setRobotPosition; // Used to determine if I am setting Robot or Waypoint position
    private int oldX, oldY;
    private CountDownTimer countDownTimer = null;
    private boolean SettingsView = false, LogView = false;
    CountDownTimer OneMinuteTimer = null;

    private long exploreMillisecondTime, exploreStartTime, exploreTimeBuff, exploreUpdateTime = 0L;
    private Handler exploreTimeHandler, sendStartCommandToPcHandler;
    private int exploreSeconds, exploreMinutes, exploreMilliSeconds ;
    private long fastestPathMillisecondTime, fastestPathStartTime, fastestPathTimeBuff, fastestPathUpdateTime = 0L;
    private Handler fastestPathTimeHandler;
    private int fastestPathSeconds, fastestPathMinutes, fastestPathMilliSeconds ;

    private SensorManager senSensorManager;
    private Sensor senAccelerometer;

    private long delayTime;

    private int[]IDString = new int[4];

    private ArrayList<int[]> imageIDarray = new ArrayList<int[]>();
    private String mdfObString = "";

    Vector fastestPath= new Vector();

    Menu menu;
    ArenaView arenaview;

    private Runnable sendStartExploreToPC = new Runnable() {
        @Override
        public void run() {
            BluetoothService.getInstance().sendText(Protocol.PC_MESSAGE_HEADER + "exs", MainActivity.this);
        }
    };

    private Runnable sendStartFastestPathToPC = new Runnable() {
        @Override
        public void run() {
            BluetoothService.getInstance().sendText(Protocol.PC_MESSAGE_HEADER + "fps", MainActivity.this);
        }
    };

    private Runnable exploreRunnable = new Runnable() {
        @Override
        public void run() {
            exploreMillisecondTime = SystemClock.uptimeMillis() - exploreStartTime;
            exploreUpdateTime = exploreTimeBuff + exploreMillisecondTime;
            exploreSeconds = (int) (exploreUpdateTime / 1000);
            exploreMinutes = exploreSeconds / 60;

            String strExploreMinutes = "";
            if(exploreMinutes < 10) {
                strExploreMinutes = "0" + String.valueOf(exploreMinutes);
            } else {
                strExploreMinutes = String.valueOf(exploreMinutes);
            }

            exploreSeconds = exploreSeconds % 60;
            exploreMilliSeconds = (int) (exploreUpdateTime % 1000);

            //specify number of digits displayed
            exploreTimeTV.setText("" + strExploreMinutes + ":"
                    + String.format("%02d", exploreSeconds) + ":"
                    + String.format("%03d", exploreMilliSeconds));

            exploreTimeHandler.postDelayed(this, 0);
        }
    };

    private Runnable fastestPathRunnable = new Runnable() {
        @Override
        public void run() {
            fastestPathMillisecondTime = SystemClock.uptimeMillis() - fastestPathStartTime;
            fastestPathUpdateTime = fastestPathTimeBuff + fastestPathMillisecondTime;
            fastestPathSeconds = (int) (fastestPathUpdateTime / 1000);
            fastestPathMinutes = fastestPathSeconds / 60;

            String strFastestPathMinutes = "";
            if(fastestPathMinutes < 10) {
                strFastestPathMinutes = "0" + String.valueOf(fastestPathMinutes);
            } else {
                strFastestPathMinutes = String.valueOf(fastestPathMinutes);
            }

            fastestPathSeconds = fastestPathSeconds % 60;

            fastestPathMilliSeconds = (int) (fastestPathUpdateTime % 1000);

            fastestTimeTV.setText("" + strFastestPathMinutes + ":"
                    + String.format("%02d", fastestPathSeconds) + ":"
                    + String.format("%03d", fastestPathMilliSeconds));

            fastestPathTimeHandler.postDelayed(this, 0);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialize the ArenaView
        arenaview = findViewById(R.id.arenaView);
        arenaview.setupArena(new Arena(this));

        // Initialize toolbar
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        btIcon = findViewById(R.id.icon_bt);
        btStatus = findViewById(R.id.status_bt);
        SendArenabutton = findViewById(R.id.SendArenaInfo);
        // Sensor
        senSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        senAccelerometer = senSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        // set Touch Listener for the toolbar
        toolbar.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                // Open Bluetooth Scanning Dialog
                if(motionEvent.getAction() == MotionEvent.ACTION_DOWN){
                    if (mBluetoothAdapter == null) {
                        // Device doesn't support Bluetooth
                        Toast.makeText(getBaseContext(), "Sorry, your device does not support Bluetooth", Toast.LENGTH_SHORT).show();
                    } else {
                        if (!mBluetoothAdapter.isEnabled()) {
                            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
                        } else {
                            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                                // Check if ACCESS_COARSE_LOCATION is granted
                                if (ContextCompat.checkSelfPermission(MainActivity.this,
                                        Manifest.permission.ACCESS_COARSE_LOCATION)
                                        != PackageManager.PERMISSION_GRANTED) {
                                    // request for ACCESS_COARSE_LOCATION since it is NOT granted
                                    ActivityCompat.requestPermissions(MainActivity.this,
                                            new String[]{Manifest.permission.ACCESS_COARSE_LOCATION},
                                            REQUEST_ALLOW_COARSE_LOCATION);
                                } else {
                                    showBTDialog();
                                }
                            } else {
                                showBTDialog();
                            }
                        }
                    }
                    return true;
                }
                return false;
            }
        });

        // Initialize Navigation Drawer
        DrawerLayout drawer = findViewById(R.id.drawer_layout);
        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        drawer.addDrawerListener(toggle);
        toggle.syncState();

        // Initialize Navigation Drawer Items
        navigationView = findViewById(R.id.nav_view);
        // Set click listener to Navigation Drawer Items
        navigationView.setNavigationItemSelectedListener(this);

        // Get the default bluetooth adapter
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        BluetoothService btc = (BluetoothService) getLastCustomNonConfigurationInstance();

        if (btc != null)
            btService = btc;

        // Register for broadcasts when a bluetooth device is discovered.
        IntentFilter filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        registerReceiver(mReceiver, filter);

        // Register for broadcasts when a bluetooth device is pairing.
        IntentFilter intent = new IntentFilter(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        registerReceiver(mPairReceiver, intent);

        // Initialize Setup Tab Button
        RelativeLayout btnSetup = findViewById(R.id.btnSetup);
        // Set click listener to Setup Tab Button
        btnSetup.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                if(vdDialog == null) {
                    vdDialog = new ViewDialog(MainActivity.this);
                }
                vdDialog.showDialog("Default Setup Configuration", "");
            }
        });

        // Initialize Explore Tab TextView
        exploreTV = findViewById(R.id.txtExplore);
        exploreTimeTV = findViewById(R.id.tvExploreTimeValue);

        // Initialize Explore Tab Button
        btnExplore = findViewById(R.id.btnExplore);
        // Set click listener to Explore Tab Button
        btnExplore.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                if(exploreTV.getText().equals("Start Explore")) {
                    ctrlFragment.hideController(false);
                    btnFastestPath.setEnabled(true);

                    /*
                    // Format of message: robot x-coord,robot y-coord,robot dir,waypt x-coord,waypt y-coord
                    StringBuilder stringBuilder = new StringBuilder();
                    // Add Communication Protocol Header
                    stringBuilder.append(Protocol.PC_MESSAGE_HEADER);
                    // Add Robot x-coord
                    stringBuilder.append(String.valueOf(arenaview.getArena().getRobot().getYPos()-1));
                    // Add separator ,
                    stringBuilder.append(",");
                    // Add Robot y-coord
                    stringBuilder.append(String.valueOf(arenaview.getArena().getRobot().getXPos()-1));
                    // Add separator ,
                    stringBuilder.append(",");
                    // Add Robot Direction
                    if(arenaview.getArena().getRobot().getDirection().equals(Robot.Direction.NORTH)) {
                        stringBuilder.append("0");
                    } else if(arenaview.getArena().getRobot().getDirection().equals(Robot.Direction.EAST)) {
                        stringBuilder.append("90");
                    } else if(arenaview.getArena().getRobot().getDirection().equals(Robot.Direction.SOUTH)) {
                        stringBuilder.append("180");
                    } else if(arenaview.getArena().getRobot().getDirection().equals(Robot.Direction.WEST)) {
                        stringBuilder.append("270");
                    }
                    // Add separator ,
                    stringBuilder.append(",");
                    // Add waypoint x-coord
                    stringBuilder.append(String.valueOf(arenaview.getArena().getWayPoint().getYPos()));
                    // Add separator ,
                    stringBuilder.append(",");
                    // Add waypoint y-coord
                    stringBuilder.append(String.valueOf(arenaview.getArena().getWayPoint().getXPos()));
                    */

                    //Send the text to bluetooth
                    BluetoothService.getInstance().sendText(Protocol.START_EXPLORATION, MainActivity.this);

                    //Send waypoint
                    //String wpString="yy,"+(arenaview.getArena().getWayPoint().getYPos()+1)+","+Math.abs(21-arenaview.getArena().getWayPoint().getXPos()-1);
                    //byte[] transmission= wpString.getBytes();
                    //btService.write(transmission);


                    exploreTV.setText("Stop Explore");
                    // Start Explore Timer
                    exploreStartTime = SystemClock.uptimeMillis();
                    if(exploreTimeHandler == null) {
                        exploreTimeHandler = new Handler();
                    }
                    if(sendStartCommandToPcHandler == null) {
                        sendStartCommandToPcHandler = new Handler();
                    }
                    exploreTimeHandler.postDelayed(exploreRunnable, 0);


                } else {
                    ctrlFragment.hideController(true);
                    btnFastestPath.setEnabled(true);
                    exploreTV.setText("Start Explore");

                    UpdateImageOnGrid1(); // FINAL IMAGE UPDATE, REMOVE IT IF FAULTY

                    // Pause Explore Timer
                    exploreTimeBuff += exploreMillisecondTime;

                    exploreTimeHandler.removeCallbacks(exploreRunnable);
                    exploreMillisecondTime = 0L;
                    exploreStartTime = 0L;
                    exploreTimeBuff= 0L;
                    exploreUpdateTime = 0L;
                    //BluetoothService.getInstance().sendText(Protocol.PC_MESSAGE_HEADER + "exStop", MainActivity.this);

                    OneMinuteTimer = new CountDownTimer(60000,1000){

                        @Override
                        public void onTick(long l) {
                            oneMinuteTimerTV.setText(l/1000+" seconds left");
                        }
                        @Override
                        public void onFinish() {
                            oneMinuteTimerTV.setText("00:01:00");
                        }
                    }.start();
                }

                // Hide the controller

            }
        });

        // Initialize FastestPath Tab TextView
        fastestTV = findViewById(R.id.txtFastestPath);
        fastestTimeTV = findViewById(R.id.tvFastestTimeValue);
        oneMinuteTimerTV= findViewById(R.id.tv1MinTimerValue);

        // Initialize FastestPath Tab Button
        btnFastestPath = findViewById(R.id.btnFasterPath);
        // Set click listener to FastestPath Tab Button
        btnFastestPath.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                if(fastestTV.getText().equals("Start Fastest Path")) {
                    ctrlFragment.hideController(false);
                    btnExplore.setEnabled(true);

                    fastestTV.setText("Stop Fastest Path");
                    BluetoothService.getInstance().sendText(Protocol.START_FASTEST+","+arenaview.getArena().getWayPoint().getYPos()+","+String.valueOf(19-arenaview.getArena().getWayPoint().getXPos()), MainActivity.this);
                    // Start FastestPath Timer
                    fastestPathStartTime = SystemClock.uptimeMillis();
                    if(fastestPathTimeHandler == null) {
                        fastestPathTimeHandler = new Handler();
                    }
                    fastestPathTimeHandler.postDelayed(fastestPathRunnable, 0);
                    if(sendStartCommandToPcHandler == null) {
                        sendStartCommandToPcHandler = new Handler();
                    }
                    //sendStartCommandToPcHandler.postDelayed(sendStartFastestPathToPC, 0);
                } else {
                    ctrlFragment.hideController(true);
                    btnExplore.setEnabled(true);
                    fastestTV.setText("Start Fastest Path");
                    // Pause FastestPath Timer
                    fastestPathTimeBuff += fastestPathMillisecondTime;

                    fastestPathTimeHandler.removeCallbacks(fastestPathRunnable);
                    fastestPathMillisecondTime = 0L;
                    fastestPathStartTime = 0L;
                    fastestPathTimeBuff = 0L;
                    fastestPathUpdateTime = 0L;
                    //BluetoothService.getInstance().sendText(Protocol.PC_MESSAGE_HEADER + "fpStop", MainActivity.this);


                }
            }
        });

        if(ctrlFragment== null) {
            ctrlFragment = new ControlFragment();
        }
        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
        transaction.replace(R.id.fragment_container, ctrlFragment);
        transaction.commit();

        activateIdleCountDownTimer();
    }
    public void UpdateImageOnGrid1() // BELLO1
    {
        Arena.GridCell gridmap[][] = arenaview.getArena().getGridMap();
        try {
            if (!imageIDarray.isEmpty()) {
                for (int i = 0; i < imageIDarray.size(); i++) {
                    int maxLocation = 3; // CHANGE HERE TO MOVE MAXIMUM DISTANCE TO ADD IMAGE
                    int[] imageId = imageIDarray.get(i);
                    int id = imageId[0];
                    int x = imageId[1];
                    int y = imageId[2];
                    int direction = imageId[3];
                    int xpos = Math.abs(21-y)-2;
                    int ypos = x;
                    //BluetoothService.getInstance().sendText(String.valueOf(id)+String.valueOf(x)+String.valueOf(y)+"a"+"\n", MainActivity.this);
                    if (gridmap[xpos][ypos].equals(Arena.GridCell.FREE_SPACE)){
                        //BluetoothService.getInstance().sendText(String.valueOf(id)+String.valueOf(x)+String.valueOf(y)+"b"+"\n", MainActivity.this);
                        while (maxLocation !=0 && gridmap[xpos][ypos].equals(Arena.GridCell.FREE_SPACE)){
                            if (direction == 0){
                                x = x-1;
                            }
                            else if (direction == 1){
                                y = y+1;
                            }
                            else if (direction == 2){
                                x = x+1;
                            }
                            else if (direction == 3){
                                y = y-1;
                            }
                            xpos = Math.abs(21-y)-2;
                            ypos = x;
                            maxLocation = maxLocation -1 ;
                        }
                        if (maxLocation != 0){
                            //BluetoothService.getInstance().sendText("here", MainActivity.this);
                            //BluetoothService.getInstance().sendText(String.valueOf(id)+String.valueOf(x)+String.valueOf(y)+"c"+"\n", MainActivity.this);
                            arenaview.getArena().getArrIDBlock(id - 1).setPosition(x + 1, y + 1);
                            int trans_x = Math.abs(y - 20);
                            int trans_y = x + 1;
                            for (int j = 0; j < 15; j++) {
                                if (j != id - 1 && (arenaview.getArena().getArrIDBlock(i).getXPos() == trans_x) && (arenaview.getArena().getArrIDBlock(i).getYPos() == trans_y)) {
                                    arenaview.getArena().getArrIDBlock(i).setPosition(18, 1);
                                    arenaview.getArena().getArrIDBlock(i).resetRender();
                                }
                            }
                            arenaview.getArena().getArrIDBlock(id - 1).setRender();
                            arenaview.invalidate();
                        } //update image

                    }
                }
                ctrlFragment.resetIDString();
                for (int i = 0; i < 15; i++) {
                    if (arenaview.getArena().getArrIDBlock(i).getRender()) {
                        String output = (i + 1) + "," + (arenaview.getArena().getArrIDBlock(i).getYPos() - 1) + "," + (20 - arenaview.getArena().getArrIDBlock(i).getXPos());
                        ctrlFragment.setTvStatus(output);
                    }

                }
            }
        }
        catch(Exception ex)
        {

        }
    }
    public void UpdateImageOnGrid() // BELLO
    {
        clearAllBlocks();
        int maxLocation = 3; // CHANGE HERE TO MOVE MAXIMUM DISTANCE TO ADD IMAGE
        try {
            if (!imageIDarray.isEmpty()) {


                int num = (Integer.parseInt(mdfObString, 16));
                String gridBinary = Integer.toBinaryString(num); //CONVERT HEX TO DECIMAL


                String[] mdfArray = gridBinary.split("");

                for (int i = 0; i < imageIDarray.size(); i++) {
                    int[] imageId = imageIDarray.get(i);
                    int x = 0, y = 0;
                    int location = imageId[1] + (15 * imageId[2]); //GET OBSTACLE OF THE CURRENT STRING
                    int count = 0;
                    boolean change = false;
                    while (count < maxLocation) //means no obstacle
                    {
                        if (mdfArray[location]=="1")
                        {
                            if (count != 0) // IF COUNT IS NOT 0, IMPLIES FIRST COOD GIVEN IS CORRECT AND DON'T NEED TO CHANGE.
                            {
                                x = location % 15;
                                y = location / 15;
                                change = true;
                            }
                            break;
                        }
                        // NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3
                        if (imageId[3]==0) //MEANS ROBOT FACING TOP
                        {
                            int y_loc = location / 15;
                            if (y_loc >= 19) // MEANS AT MOST TOP ALREADY
                            {
                                break;
                            }
                            location = location + 15; // ADD 15 TO ADD Y
                        }
                        else if (imageId[3]==1){ // MEANS ROBOT FACING RIGHT
                            int x_loc = location % 15; // GET CURRENT X POSITION
                            if (x_loc >= 14) //MEANS AT THE EDGE ALREADY, CANNOT ADD
                            {
                                break;
                            }
                            location = location + 1; // ADD 1 TO X
                        }
                        else if (imageId[3]==2) // MEANS ROBOT FACING DOWNWARDS
                        {
                            int y_loc = location / 15;
                            if (y_loc == 0) // MEANS AT MOST BOTTOM ALREADY
                            {
                                break;
                            }
                            location = location - 15; // MINUS 15 TO ADD Y
                        }
                        else if (imageId[3]==3) // MEANS ROBOT FACING LEFT
                        {
                            int x_loc = location % 15; // GET CURRENT X POSITION
                            if (x_loc == 0) //MEANS AT THE EDGE ALREADY, CANNOT ADD
                            {
                                break;
                            }
                            location = location - 1;
                        }
                        count++;
                    }

                    if (change==true)
                    {
                        drawBlock(imageId[0], x , y);
                    }
                }
            }
        }
        catch(Exception ex)
        {

        }

    }

    public void clearAllBlocks()
    {
        for (int i = 0; i < 15; i++) {
            // BluetoothService.getInstance().sendText(Integer.toString(i) + " | " + Integer.toString(arenaview.getArena().getArrIDBlock(i).getXPos()) + "| " + IDCommand[2] + "|\n", this);
            arenaview.getArena().getArrIDBlock(i).setPosition(18, 1);
            arenaview.getArena().getArrIDBlock(i).resetRender();
        }
    }

    public void drawBlock(int imageID, int x, int y)
    {
        arenaview.getArena().getArrIDBlock(imageID).setPosition(x + 1, y + 1);

        int trans_x = Math.abs(x - 20);
        int trans_y = y + 1;
        for (int i = 0; i < 15; i++) {
            if (i != imageID && (arenaview.getArena().getArrIDBlock(i).getXPos() == trans_x) && (arenaview.getArena().getArrIDBlock(i).getYPos() == trans_y)) {
                // BluetoothService.getInstance().sendText(Integer.toString(i) + " | " + Integer.toString(arenaview.getArena().getArrIDBlock(i).getXPos()) + "| " + IDCommand[2] + "|\n", this);
                arenaview.getArena().getArrIDBlock(i).setPosition(18, 1);
                arenaview.getArena().getArrIDBlock(i).resetRender();
            }
        }

        arenaview.getArena().getArrIDBlock(imageID).setRender();
        arenaview.invalidate();
    }

    // Shortcut to set the Robot or Waypoint follow by it's X and Y position
    public void setRobotPosition (boolean s) {
        this.setRobotPosition = s;
        if (s){
            oldX = arenaview.getArena().getRobot().getXPos();
            oldY = arenaview.getArena().getRobot().getYPos();
        }
        else{
            oldX = arenaview.getArena().getWayPoint().getXPos();
            oldY = arenaview.getArena().getWayPoint().getYPos();
        }


    }









    // Set inputWayPointPosition
    public void inputWayPointPosition(int type) {
        if (arenaview != null && arenaview.getArena() != null) {
            enableInputPosition(true, type);
//            if(arenaview.getArena().isReset()) {
//                enableInputPosition(true, type);
//            }
//            else {
//                Operation.showToast(this, "Please reset your arena first");
//            }
        }
    }

    // Enable input position for way point
    private void enableInputPosition(boolean b, int t) {
        if (arenaview != null && arenaview.getArena() != null) {
            arenaview.setInputPositionEnabled(b, t);
        }

        if(t == 0 || t == 1) {
            for (int i = 0; i < menu.size(); i ++)
                menu.getItem(i).setVisible(b);
        }
        else if(t == 2) {
            menu.getItem(1).setVisible(b);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_done:
                enableInputPosition(false, 0);

                // setting robot position done
                if (setRobotPosition) {
                    if (oldX != arenaview.getArena().getRobot().getXPos() ||
                            oldY != arenaview.getArena().getRobot().getYPos()) {
                        String newX = String.valueOf(mod((arenaview.getArena().getRobot().getYPos() - 15), 15));
                        String newY = String.valueOf(mod((19 - arenaview.getArena().getRobot().getXPos()), 20));
                        BluetoothService.getInstance().sendText("Robot position changed from X: " +
                                String.valueOf(mod((oldY - 15), 15)) + ", Y: " + String.valueOf(mod((19 - oldX), 20)) + " to X: " +
                                newX + ", Y: " + newY + "\n", this);
                        BluetoothService.getInstance().sendText("coordinate (" +
                                String.valueOf(arenaview.getArena().getRobot().getYPos()) + "," +
                                String.valueOf(19 - arenaview.getArena().getRobot().getXPos()) + ")", this);
                    } else {
                        Operation.showToast(this, "No change to the position of the Robot.");
                    }
                }

                // setting waypoint position done
               /* else {
                    if(oldX != arenaview.getArena().getWayPoint().getXPos() ||
                            oldY != arenaview.getArena().getWayPoint().getYPos()) {
                        String newX = String.valueOf(mod((arenaview.getArena().getWayPoint().getYPos()-15), 15));
                        String newY = String.valueOf(mod((19-arenaview.getArena().getWayPoint().getXPos()), 20));
                        BluetoothService.getInstance().sendText("Waypoint position changed from X: " +
                                String.valueOf(mod((oldY-15), 15)) + ", Y: " + String.valueOf(mod((19-oldX), 20)) + " to X: " +
                                newX + ", Y: " + newY + "\n", this);
                    } else {
                        Operation.showToast(this, "No change to the position of the Robot.");
                    }
                }

                */
                break;

            case R.id.menu_rotate:
                arenaview.getArena().getRobot().turnRight();
                arenaview.invalidate();
                break;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        this.menu = menu;
        getMenuInflater().inflate(R.menu.input_waypoint_position, menu);

        for (int i = 0; i < menu.size(); i++){
            menu.getItem(i).setVisible(false);
            Drawable drawable = menu.getItem(i).getIcon();

            if (drawable != null) {
                drawable.mutate();
                drawable.setColorFilter(getResources().getColor(R.color.white), PorterDuff.Mode.SRC_ATOP);
            }
        }
        return super.onCreateOptionsMenu(menu);
    }

    // Create a BroadcastReceiver for ACTION_FOUND.
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                // Discovery has found a device. Get the BluetoothDevice
                // object and its info from the Intent.
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                String deviceName = device.getName();
                String deviceHardwareAddress = device.getAddress(); // MAC address
                btDialog.updateAvailableListView(deviceName + "\n" + deviceHardwareAddress, device);
            } else if (BluetoothDevice.ACTION_ACL_CONNECTED.equals(action)) {
                Toast.makeText(context, "Connected", Toast.LENGTH_SHORT).show();
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                //Done searching
                Toast.makeText(context, "Done Searching", Toast.LENGTH_SHORT).show();
            }
            else if (BluetoothDevice.ACTION_ACL_DISCONNECT_REQUESTED.equals(action)) {
                //Device is about to disconnect
                Toast.makeText(context, "About to disconnect", Toast.LENGTH_SHORT).show();
            }
            else if (BluetoothDevice.ACTION_ACL_DISCONNECTED.equals(action)) {
                //Device has disconnected
                Toast.makeText(context, "Disconnected", Toast.LENGTH_SHORT).show();
            }
        }
    };

    // BroadcastReceiver for pairing Bluetooth devices
    private final BroadcastReceiver mPairReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals(action)) {
                final int state        = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, BluetoothDevice.ERROR);
                final int prevState    = intent.getIntExtra(BluetoothDevice.EXTRA_PREVIOUS_BOND_STATE, BluetoothDevice.ERROR);

                if (state == BluetoothDevice.BOND_BONDED && prevState == BluetoothDevice.BOND_BONDING) {
                    // Device Paired Successfully
                    btDialog.updatePairedSuccess();
                } else if (state == BluetoothDevice.BOND_NONE && prevState == BluetoothDevice.BOND_BONDED){
                    // Device Unpaired Successfully
                    btDialog.updateUnpairedSuccess();
                }
            }
        }
    };

    private BroadcastReceiver messageReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // Get extra data included in the Intent
            Log.d(Config.log_id, "Broadcast Message received");
            Log.d(Config.log_id, "Message type: " + intent.getExtras().getInt(Protocol.MESSAGE_TYPE));
            Message msg = Message.obtain();
            msg.what = intent.getExtras().getInt(Protocol.MESSAGE_TYPE);
            msg.setData(intent.getExtras());

            if (intent.getExtras().getInt(Protocol.MESSAGE_ARG1, -99) != -99) {
                msg.arg1 = intent.getExtras().getInt(Protocol.MESSAGE_ARG1, -99);
            }

            handleMessage(msg);
        }
    };

    @Override
    public void onBackPressed() {
       DrawerLayout drawer = findViewById(R.id.drawer_layout);
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else if (debugFragment != null) {
            FragmentManager fm = this.getSupportFragmentManager();
            if(fm.getBackStackEntryCount()>0) {
                fm.popBackStack();
            }
            debugFragment = null;
            navigationView.getMenu().getItem(2).setChecked(false);
        }
        else if (LogView)
        {
            FragmentManager fm = this.getSupportFragmentManager();
            if(fm.getBackStackEntryCount()>0) {
                fm.popBackStack();
            }
            LogView = false;
        }
        else {
            //goToHomeScreen();
            super.onBackPressed();
        }


    }

    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle navigation view item clicks here.
        int id = item.getItemId();

        if (id == R.id.nav_home){

            //if (debugFragment != null) {
                FragmentManager fm = this.getSupportFragmentManager();
                if (fm.getBackStackEntryCount() > 0) {
                    fm.popBackStack();
                }
                debugFragment = null;
                item.setChecked(false);
            /*}
            else
            {
                Intent intent = new Intent(this, MainActivity.class);
                startActivity(intent);
            }*/
        }
        else if (id == R.id.nav_connect) {
            showBTDialog();
        } else if (id == R.id.reset_arena) {
            for(int i=0;i<15;i++){
                arenaview.getArena().getArrIDBlock(i).resetRender();
            }
            arenaview.getArena().resetArena();
            ctrlFragment.setMDF1("");
            ctrlFragment.setMDF2("");
            ctrlFragment.resetIDString();
            arenaview.invalidate();
            exploreTimeTV.setText("00:00:00");
            fastestTimeTV.setText("00:00:00");
            OneMinuteTimer.cancel();
            oneMinuteTimerTV.setText("1 Minute Timer: 00:00:00");

            item.setChecked(false);
            if (exploreTV.getText().equals("Stop Explore")) {
                btnExplore.performClick();
            }
            if(fastestTV.getText().equals("Stop Fastest Path")) {
                btnFastestPath.performClick();
            }
        } else if (id == R.id.nav_log) {
            LogView = true;
            Intent intent = new Intent(this, LogActivity.class);
            startActivity(intent);
        } else if (id == R.id.nav_debug) {
           // if(debugFragment == null) {

                // CHANGED TO JUST REFRESHING
                debugFragment = new DebugFragment();
                FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
                transaction.replace(R.id.fragment_container, debugFragment);
                transaction.addToBackStack(null);
                transaction.commit();
            /*} else {
                FragmentManager fm = this.getSupportFragmentManager();
                if(fm.getBackStackEntryCount()>0) {
                    fm.popBackStack();
                }
                debugFragment = null;
                item.setChecked(false);
            }*/
        } else if (id == R.id.nav_settings) {

            Intent intent = new Intent(this, SettingsActivity.class);
            startActivity(intent);
        }

        DrawerLayout drawer = findViewById(R.id.drawer_layout);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        // Enable Bluetooth Request
        if (requestCode == REQUEST_ENABLE_BT) {
            // Bluetooth Enabled
            if (resultCode == RESULT_OK) {
                // Only ask for these permissions on runtime when running Android 6.0 or higher
                // Bluetooth Discovery requires Coarse Location for device running Android 6.0 or higher
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    // Check if ACCESS_COARSE_LOCATION is granted
                    if (ContextCompat.checkSelfPermission(MainActivity.this,
                            Manifest.permission.ACCESS_COARSE_LOCATION)
                            != PackageManager.PERMISSION_GRANTED) {
                        // request for ACCESS_COARSE_LOCATION since it is NOT granted
                        ActivityCompat.requestPermissions(MainActivity.this,
                                new String[]{Manifest.permission.ACCESS_COARSE_LOCATION},
                                REQUEST_ALLOW_COARSE_LOCATION);
                    } else {
                        showBTDialog();
                    }
                }
            } else {
                Toast.makeText(getBaseContext(), "You need to turn on your Bluetooth", Toast.LENGTH_SHORT).show();
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case REQUEST_ALLOW_COARSE_LOCATION: {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted
                    showBTDialog();
                } else {
                    // permission denied
                    Toast.makeText(getBaseContext(), "Bluetooth Discovery requires Coarse Location Permission\nfor device running Android 6.0 or higher", Toast.LENGTH_SHORT).show();
                }
                return;
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // unregister the ACTION_FOUND receiver.
        unregisterReceiver(mReceiver);
        unregisterReceiver(mPairReceiver);
        removeSchedulerCallBack();
        LocalBroadcastManager.getInstance(this).unregisterReceiver(messageReceiver);
        senSensorManager.unregisterListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();

        // Restore the connected bluetooth device name and address
        mConnectedDeviceName = getSharedPreferences("protocolPref", MODE_PRIVATE).getString(Protocol.DEVICE_NAME, "");
        mConnectedDeviceMac = getSharedPreferences("protocolPref", MODE_PRIVATE).getString(Protocol.DEVICE_MAC, "");
        IntentFilter intentFilter = new IntentFilter("BTLocalBroadcast");
        LocalBroadcastManager.getInstance(this).registerReceiver(messageReceiver, intentFilter);
        // Performing this check in onResume() covers the case in which BT was
        // not enabled during onStart(), so we were paused to enable it...
        // onResume() will be called when ACTION_REQUEST_ENABLE activity returns.

        if (btService != null) {
            // Only if the state is STATE_NONE, do we know that we haven't started already
            // Start the Bluetooth chat services

            if (btService.getConnectionState() == BluetoothService.STATE_NONE) {
                // Start the Bluetooth chat services
                btService.initialiseService();
            }

            if (btService.getConnectionState() == BluetoothService.STATE_LISTEN) {
                if(btStatus == null || btIcon == null) {
                    btIcon = (ImageView) findViewById(R.id.icon_bt);
                    btStatus = (TextView) findViewById(R.id.status_bt);
                }
                btIcon.setImageResource(R.drawable.disconnected_bluetooth);
                btStatus.setText(this.getString(R.string.disconnected_bt));
                if(btDialog == null) {
                    btDialog = new BluetoothDialog(MainActivity.this, btService);
                }
                btDialog.updateList(mConnectedDeviceName, mConnectedDeviceMac, false);

            } else if (btService.getConnectionState() == BluetoothService.STATE_CONNECTED) {
                if(btStatus == null || btIcon == null) {
                    btIcon = (ImageView) findViewById(R.id.icon_bt);
                    btStatus = (TextView) findViewById(R.id.status_bt);
                }
                btIcon.setImageResource(R.drawable.connected_bluetooth);
                btStatus.setText("Connected to " + mConnectedDeviceName);
                if(btDialog == null) {
                    btDialog = new BluetoothDialog(MainActivity.this, btService);
                }
                btDialog.updateList(mConnectedDeviceName, mConnectedDeviceMac, true);
            }
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        // If BT is not on, request that it be enabled.

        if (mBluetoothAdapter != null) {
            if (!mBluetoothAdapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);

            } else if (btService == null) {
                Log.e(Config.log_id, "on start bluetooth service restart");
                // Initialize the BluetoothChatService to perform bluetooth connections
                btService = BluetoothService.getInstance();
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();

        LocalBroadcastManager.getInstance(this).unregisterReceiver(messageReceiver);
        removeSchedulerCallBack();

        // Save the connected bluetooth device name and address
        SharedPreferences.Editor editor = getSharedPreferences(Protocol.PROTOCOLPREF, MODE_PRIVATE).edit();
        editor.putString(Protocol.DEVICE_NAME, mConnectedDeviceName);
        editor.putString(Protocol.DEVICE_MAC, mConnectedDeviceMac);
        editor.commit();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putString(Protocol.DEVICE_NAME, mConnectedDeviceName);
        outState.putString(Protocol.DEVICE_MAC, mConnectedDeviceMac);
        //savedInstanceState.putBoolean("isAccelerometerEnabled", isAccelerometerEnabled);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        mConnectedDeviceName = savedInstanceState.getString(Protocol.DEVICE_NAME);
        mConnectedDeviceMac = savedInstanceState.getString(Protocol.DEVICE_MAC);
    }

    @Override
    public Object onRetainCustomNonConfigurationInstance() {
        return  btService;
    }

    private void showBTDialog() {
        if(btDialog == null) {
            if(btService == null) {
                // Initialize the BluetoothChatService to perform bluetooth connections
                btService = BluetoothService.getInstance();
            }
            btDialog = new BluetoothDialog(MainActivity.this, btService);
        }

        btDialog.showDialog("Scan for Bluetooth Devices", btService);
    }

    public void handleMessage(Message msg) {
        switch (msg.what) {
            case Protocol.MESSAGE_STATE_CHANGE:
                switch (msg.arg1) {
                    case BluetoothService.STATE_CONNECTED:
                        btIcon.setImageResource(R.drawable.connected_bluetooth);
                        btStatus.setText("Connected to " + mConnectedDeviceName);
                        if(btDialog == null) {
                            if (btService == null) {
                                // Initialize the BluetoothChatService to perform bluetooth connections
                                btService = BluetoothService.getInstance();
                            }
                            btDialog = new BluetoothDialog(MainActivity.this, btService);
                        }
                        btDialog.updateList(mConnectedDeviceName, mConnectedDeviceMac, true);
                        break;

                    case BluetoothService.STATE_CONNECTING:
                        btIcon.setImageResource(R.drawable.disconnected_bluetooth);
                        if(mConnectedDeviceName != null) {
                            writeToLog("Connecting to\n" + mConnectedDeviceName + ", " + mConnectedDeviceMac);
                        }
                        btStatus.setText("Connecting to " + mConnectedDeviceName);
                        break;

                    case BluetoothService.STATE_LISTEN:
                    case BluetoothService.STATE_NONE:
                        btIcon.setImageResource(R.drawable.disconnected_bluetooth);
                        btStatus.setText(this.getString(R.string.disconnected_bt));
                        if(btDialog == null) {
                            if(btService == null) {
                                // Initialize the BluetoothChatService to perform bluetooth connections
                                btService = BluetoothService.getInstance();
                            }
                            btDialog = new BluetoothDialog(MainActivity.this, btService);
                        }

                        if (msg.arg1 == BluetoothService.STATE_NONE) {
                            writeToLog("Disconnected from\n" + mConnectedDeviceName + ", " + mConnectedDeviceMac);
                        }
                        btDialog.updateList(mConnectedDeviceName, mConnectedDeviceMac, false);
                        break;
                }

                break;







            case Protocol.MESSAGE_WRITE:
                byte[] writeBuf = msg.getData().getByteArray(Protocol.MESSAGE_BUFFER);
                String writeMessage = new String(writeBuf);
                writeToLog("Message sent to\n" + mConnectedDeviceName + ", " + mConnectedDeviceMac + ": \n" + writeMessage);
                break;

            case Protocol.MESSAGE_READ:
                int bytes = msg.getData().getInt(Protocol.MESSAGE_BYTES);
                byte[] buffer = msg.getData().getByteArray(Protocol.MESSAGE_BUFFER);
                String readMessage = new String(buffer, 0, bytes).trim();
                writeToLog("Message from\n" + mConnectedDeviceName + ", " + mConnectedDeviceMac + ":\n" + readMessage);
                if(debugFragment != null) {
                    debugFragment.setReceiveET(readMessage);
                }

                // GridMap Update String from PC
                /*
                if(readMessage.equalsIgnoreCase("exr")) {
                    //sendStartCommandToPcHandler.postDelayed(sendStartExploreToPC, 0);
                }

                else if(readMessage.startsWith("{\"robot\":")) {
                    readMessage = readMessage.replace("}{", "}, {");
                    readMessage = readMessage.replace("} {", "}, {");
                    readMessage = "[ " + readMessage + " ]";
                    String robot = "";
                    try {
                        JSONArray json = new JSONArray(readMessage);
                        JSONObject obj = json.getJSONObject(json.length()-1);
                        robot = obj.getString("robot").trim();
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                    //message[0]:part1, message[1]:part2, message[2]:robot y-coord count from 1 from below,
                    // message[3]:robot x-coord count from 1 from left, message[4]: robot head direction
                    String[] message = robot.split(",");

                    arenaview.getArena().getRobot().setPosition(Integer.valueOf(message[2]), Integer.valueOf(message[3]));

                    switch((Integer.parseInt(message[4].trim()))) {
                        case 0: arenaview.getArena().getRobot().setDirection(Robot.Direction.NORTH);
                            break;
                        case 90: arenaview.getArena().getRobot().setDirection(Robot.Direction.EAST);
                            break;
                        case 180: arenaview.getArena().getRobot().setDirection(Robot.Direction.SOUTH);
                            break;
                        case 270: arenaview.getArena().getRobot().setDirection(Robot.Direction.WEST);
                            break;
                        default:
                    }
                    handlePCGridUpdate(message[0], message[1]);

                    if(ctrlFragment.getAutoStatus()) {
                        arenaview.invalidate();
                    }
                }
                */
                /*
                else if(readMessage.equalsIgnoreCase("exploreComplete")) {
                    if(exploreTV.getText().equals("Stop Explore")) {
                        btnExplore.performClick();
                    }
                }
                else if(readMessage.equalsIgnoreCase("S")) {
                    if(fastestTV.getText().equals("Stop Fastest Path")) {
                        btnFastestPath.performClick();
                    }
                }
                */
                // Now modified to receive string for grid update(mdfstring1, mdfstring2), robot position update(x,y), robot direction(0,1,2,3)
                else if(readMessage.startsWith("grid")) {
                    try{
                        Log.d(TAG, "handleMessage: actual sent message:"+readMessage);
                        //BluetoothService.getInstance().sendText(readMessage, this);

                        String mdfStrings[]=readMessage.split(",");
                        mdfObString = mdfStrings[2];
                        arenaview.getArena().getRobot().setPosition(Math.abs(21-Integer.valueOf(mdfStrings[4]))-2,Integer.valueOf(mdfStrings[3]));
                        //BluetoothService.getInstance().sendText(String.valueOf(arenaview.getArena().getRobot().getXPos())+String.valueOf(arenaview.getArena().getRobot().getYPos()), this);

                        Robot.Direction direction = arenaview.getArena().getRobot().getDirection();



                        if((direction==Robot.Direction.NORTH) && (Integer.parseInt(mdfStrings[5].trim())) == 1){
                            ctrlFragment.setStatus(Protocol.STATUS_TURN_RIGHT);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.NORTH) && (Integer.parseInt(mdfStrings[5].trim())) == 3){

                            ctrlFragment.setStatus(Protocol.STATUS_TURN_LEFT);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.EAST) && (Integer.parseInt(mdfStrings[5].trim())) == 2){
                            ctrlFragment.setStatus(Protocol.STATUS_TURN_RIGHT);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.EAST) && (Integer.parseInt(mdfStrings[5].trim())) == 0){
                            ctrlFragment.setStatus(Protocol.STATUS_TURN_LEFT);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.SOUTH) && (Integer.parseInt(mdfStrings[5].trim())) == 3){
                            ctrlFragment.setStatus(Protocol.STATUS_TURN_RIGHT);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.SOUTH) && (Integer.parseInt(mdfStrings[5].trim())) == 1){
                            ctrlFragment.setStatus(Protocol.STATUS_TURN_LEFT);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.WEST) && (Integer.parseInt(mdfStrings[5].trim())) == 0){
                            ctrlFragment.setStatus(Protocol.STATUS_TURN_RIGHT);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.WEST) && (Integer.parseInt(mdfStrings[5].trim())) == 2){
                            ctrlFragment.setStatus(Protocol.STATUS_TURN_LEFT);
                            activateIdleCountDownTimer();
                        }

                        else if((direction==Robot.Direction.NORTH) && (Integer.parseInt(mdfStrings[5].trim())) == 0){
                            ctrlFragment.setStatus(Protocol.STATUS_FORWARD);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.EAST) && (Integer.parseInt(mdfStrings[5].trim())) == 1){
                            ctrlFragment.setStatus(Protocol.STATUS_FORWARD);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.SOUTH) && (Integer.parseInt(mdfStrings[5].trim())) == 2){
                            ctrlFragment.setStatus(Protocol.STATUS_FORWARD);
                            activateIdleCountDownTimer();
                        }
                        else if((direction==Robot.Direction.WEST) && (Integer.parseInt(mdfStrings[5].trim())) == 3){
                            ctrlFragment.setStatus(Protocol.STATUS_FORWARD);
                            activateIdleCountDownTimer();
                        }



                        switch((Integer.parseInt(mdfStrings[5].trim()))) {
                            case 0: arenaview.getArena().getRobot().setDirection(Robot.Direction.NORTH);
                                break;
                            case 1: arenaview.getArena().getRobot().setDirection(Robot.Direction.EAST);
                                break;
                            case 2: arenaview.getArena().getRobot().setDirection(Robot.Direction.SOUTH);
                                break;
                            case 3: arenaview.getArena().getRobot().setDirection(Robot.Direction.WEST);
                                break;
                            default:
                                Log.d(TAG, "handleMessage: Incorrect direction");
                        }
                        handlePCGridUpdate(mdfStrings[1], mdfStrings[2]);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }

                }
                else if(readMessage.startsWith("wpwait")) {
                    String wpString="yy,"+(arenaview.getArena().getWayPoint().getYPos())+","+Math.abs(21-arenaview.getArena().getWayPoint().getXPos()-2);
                    byte[] transmission= wpString.getBytes();
                    btService.write(transmission);
                    //btService.sendText(wpString,getApplicationContext());
                }
                else if(readMessage.startsWith(Protocol.STOP_EXPLORATION)){
                    btnExplore.performClick();


                    //String wpString="yy,"+(arenaview.getArena().getWayPoint().getYPos())+","+Math.abs(21-arenaview.getArena().getWayPoint().getXPos()-2);
                    //byte[] transmission= wpString.getBytes();
                   // btService.write(transmission);
                }


                    //Code for dealing with the fastest path
                else if(readMessage.startsWith("coord")){
                    try{
                        String mdfStrings[]=readMessage.split(",");
                        //mdfStrings[1] is the command that the PC would be sending to arduino
                        if(mdfStrings[1].startsWith("A")){
                            arenaview.getArena().getRobot().turnLeft();
                            arenaview.invalidate();
                        }
                        else if(mdfStrings[1].startsWith("D")){
                            arenaview.getArena().getRobot().turnRight();
                            arenaview.invalidate();
                        }
                        else{
                            int fwdSteps=0;
                            switch(mdfStrings[1]){
                                case("1"):
                                    fwdSteps=1;
                                    break;
                                case("2"):
                                    fwdSteps=2;
                                    break;
                                case("3"):
                                    fwdSteps=3;
                                    break;
                                case("4"):
                                    fwdSteps=4;
                                    break;
                                case("5"):
                                    fwdSteps=5;
                                    break;
                                case("6"):
                                    fwdSteps=6;
                                    break;
                                case("7"):
                                    fwdSteps=7;
                                    break;
                                case("8"):
                                    fwdSteps=8;
                                    break;
                                case("9"):
                                    fwdSteps=9;
                                    break;
                                default:
                                    Log.d(TAG, "handleMessage: Unexpected coord command sent");
                                    break;
                            }
                            for(int i=0; i<fwdSteps;i++){
                                arenaview.getArena().getRobot().moveForward(true);
                                arenaview.invalidate();
                            }
                        }

                    } catch (Exception e) {
                        e.printStackTrace();
                    }

                }



                else if(readMessage.startsWith(Protocol.STOP_FASTEST)){
                    btnFastestPath.performClick();
                }
                else if(readMessage.startsWith("ID")) {

                    readMessage = readMessage.replace(" ", ",");
                    String IDCommand[] = readMessage.split(",");

                    //Log.d(TAG, "run: current Message: " + IDCommand[0]);
                    if (IDCommand[0].compareTo("ID") == 0 || IDCommand[0].compareTo("id") == 0) {
                        //BluetoothService.getInstance().sendText(IDCommand[0] + " | " + IDCommand[1] + " | " +IDCommand[2] + " | " + IDCommand[3], this);
                        Log.d(TAG, "run: ID command registered");
                        //IDCommand[0]: ID
                        //IDCommand[1]: <idblock number>
                        //IDCommand[2]: x coordinate
                        //IDCommand[3]: y coordinate

                        //Set the ID and coordinates in the array
                        try{
                            int[]IDString = new int[4];
                            for(int i=0;i<4;i++){
                                IDString[i]= Integer.parseInt(IDCommand[i+1]);
                            }

                            imageIDarray.add(IDString);
                            //ctrlFragment.setIDString(IDString);

                            if(1<=Integer.parseInt(IDCommand[1])&&Integer.parseInt(IDCommand[2])<=15){
                                //BluetoothService.getInstance().sendText(IDCommand[2], this);
                                if(0<=Integer.parseInt(IDCommand[2])&&Integer.parseInt(IDCommand[2])<=14 && 0<=Integer.parseInt(IDCommand[3])&&Integer.parseInt(IDCommand[3])<=19) {

                                    arenaview.getArena().getArrIDBlock(Integer.parseInt(IDCommand[1]) - 1).setPosition(Integer.parseInt(IDCommand[2]) + 1, Integer.parseInt(IDCommand[3]) + 1);
                                    //BluetoothService.getInstance().sendText(arenaview.getArena().getArrIDBlock(Integer.parseInt(IDCommand[1])-1).getXPos()+ "|" + arenaview.getArena().getArrIDBlock(Integer.parseInt(IDCommand[1])-1).getYPos() + "\n" , this);
                                    int trans_x = Math.abs(Integer.parseInt(IDCommand[3]) - 20);
                                    int trans_y = Integer.parseInt(IDCommand[2]) + 1;
                                    for (int i = 0; i < 15; i++) {
                                        //BluetoothService.getInstance().sendText(Integer.toString(i+1) + "x | " + Integer.toString(arenaview.getArena().getArrIDBlock(i).getXPos()) + "| " + trans_x + "|\n", this);
                                        //BluetoothService.getInstance().sendText(Integer.toString(i+1) + "y | " + Integer.toString(arenaview.getArena().getArrIDBlock(i).getYPos()) + "| " + trans_y + "|\n", this);
                                        if ((i != Integer.parseInt(IDCommand[1]) - 1) && (arenaview.getArena().getArrIDBlock(i).getXPos() == trans_x) && (arenaview.getArena().getArrIDBlock(i).getYPos() == trans_y)) {
                                            // BluetoothService.getInstance().sendText(Integer.toString(i) + " | " + Integer.toString(arenaview.getArena().getArrIDBlock(i).getXPos()) + "| " + IDCommand[2] + "|\n", this);
                                            arenaview.getArena().getArrIDBlock(i).setPosition(18, 1);
                                            arenaview.getArena().getArrIDBlock(i).resetRender();
                                        }
                                    }

                                    arenaview.getArena().getArrIDBlock(Integer.parseInt(IDCommand[1]) - 1).setRender();
                                    arenaview.invalidate();


                                    ctrlFragment.resetIDString();

                                    for (int i = 0; i < 15; i++) {
                                        if (arenaview.getArena().getArrIDBlock(i).getRender()) {
                                            String output = (i + 1) + "," + (arenaview.getArena().getArrIDBlock(i).getYPos() - 1) + "," + (20 - arenaview.getArena().getArrIDBlock(i).getXPos());
                                            ctrlFragment.setTvStatus(output);
                                        }

                                    }
                                }
                                else
                                {
                                    Operation.showToast(getApplicationContext(), "Invalid cood received ");
                                }
                            }
                            else
                            {
                                Operation.showToast(getApplicationContext(), "Invalid Image ID received ");
                            }

                        }
                        catch(Exception e){
                            Log.d(TAG, "handleMessage: ID: Error in string format");
                        }
                    }
                }
                //Message may come from AMD Tool (which we'll ignore for the actual run since... it's useless)

//                else {
//                    // AMD sent a move forward
//                    if(readMessage.equals(Protocol.AMD_MOVE_FORWARD)) {
//                        arenaview.getArena().getRobot().moveForward(true);
//                        BluetoothService.getInstance().sendText(Protocol.AMD_MOVE_FORWARD.toString(), this);
//                        if(ctrlFragment != null) {
//                            ctrlFragment.setStatus(Protocol.STATUS_FORWARD);
//                        }
//                        arenaview.invalidate();
//                    }
//                    // AMD sent a turn left
//                    else if(readMessage.equals(Protocol.AMD_TURN_LEFT)) {
//                        arenaview.getArena().getRobot().turnLeft();
//                        BluetoothService.getInstance().sendText(Protocol.AMD_TURN_LEFT.toString(), this);
//                        if(ctrlFragment != null) {
//                            ctrlFragment.setStatus(Protocol.STATUS_TURN_LEFT);
//                        }
//                        arenaview.invalidate();
//                    }
//                    // AMD sent a turn right
//                    else if(readMessage.equals(Protocol.AMD_TURN_RIGHT)) {
//                        arenaview.getArena().getRobot().turnRight();
//                        BluetoothService.getInstance().sendText(Protocol.AMD_TURN_RIGHT.toString(), this);
//                        if(ctrlFragment != null) {
//                            ctrlFragment.setStatus(Protocol.STATUS_TURN_RIGHT);
//                        }
//                        arenaview.invalidate();
//                    }
//                    // AMD sent a reverse
//                    else if(readMessage.equals(Protocol.AMD_REVERSE)) {
//                        arenaview.getArena().getRobot().reverse(true);
//                        BluetoothService.getInstance().sendText(Protocol.AMD_REVERSE.toString(), this);
//                        if(ctrlFragment != null) {
//                            ctrlFragment.setStatus(Protocol.STATUS_REVERSE);
//                        }
//                        arenaview.invalidate();
//                    }
//
//
//
//
//
//
//
//                    // AMD sent a grid string
//                    else if(readMessage.startsWith("{\"grid\" :")){
//                        //BluetoothService.getInstance().sendText(readMessage, this);
//                        handleGridUpdate(readMessage.substring(11, readMessage.length()-2));
//                    }
//                    // For Testing AMD Status Checkpoint
//                    else if(readMessage.startsWith("{\"status\":")){
//                        ctrlFragment.setStatus(readMessage.substring(11, readMessage.length()-2));
//                    }
//                    // For Updating of AMD Robot position
//                    else if(readMessage.startsWith("{\"robotPosition\" :")) {
//                       // BluetoothService.getInstance().sendText(readMessage, this);
//                        readMessage = readMessage.substring(20, readMessage.length()-2);
//                        String[] message = readMessage.split(",");
//                        arenaview.getArena().getRobot().setPosition(Integer.parseInt(message[1].trim()) +1, Integer.parseInt(message[0].trim()) + 1);
//                        switch((Integer.parseInt(message[2].trim()))) {
//                            case 0: arenaview.getArena().getRobot().setDirection(Robot.Direction.NORTH);
//                                break;
//                            case 90: arenaview.getArena().getRobot().setDirection(Robot.Direction.EAST);
//                                break;
//                            case 180: arenaview.getArena().getRobot().setDirection(Robot.Direction.SOUTH);
//                                break;
//                            case 270: arenaview.getArena().getRobot().setDirection(Robot.Direction.WEST);
//                                break;
//                            default:
//                        }
//                        if(ctrlFragment.getAutoStatus()) {
//                            arenaview.invalidate();
//                        }
//                    }
//                }

                break;

            case Protocol.MESSAGE_DEVICE_NAME:
                // save the connected device's name
                mConnectedDeviceName = msg.getData().getString(Protocol.DEVICE_NAME);
                mConnectedDeviceMac = msg.getData().getString(Protocol.DEVICE_MAC);
                if(btService.getConnectionState() == BluetoothService.STATE_CONNECTED) {
                    writeToLog("Connected to\n" + mConnectedDeviceName + ", " + mConnectedDeviceMac);
                    Operation.showToast(getApplicationContext(), "Connected to " + mConnectedDeviceName);
                }
                break;

            case Protocol.MESSAGE_TOAST:
                Operation.showToast(getApplicationContext(), msg.getData().getString(Protocol.TOAST));
                break;
        }
    }
    public void removeSchedulerCallBack() {
        try {
            handlerAutoUpdate.removeCallbacksAndMessages(null);
        } catch (Exception e) {
            Log.e(Config.log_id, e.getMessage());
        }
    }

    public void setBtService(BluetoothService btService) {
        this.btService = btService;
    }

    public String getConnectedDeviceName() {
        return this.mConnectedDeviceName;
    }

    public String getConnectedDeviceMac() { return this.mConnectedDeviceMac; }

    public ArenaView getArenaview() {
        return arenaview;
    }

    private void writeToLog(String content){
        File fileDir = new File(getFilesDir(),Protocol.LOG_FILE_DIR);
        if(!fileDir.exists()){
            fileDir.mkdir();
        }

        try{
            File logFile = new File(fileDir, Protocol.LOG_FILE_NAME);
            FileWriter writer = new FileWriter(logFile, true);
            SimpleDateFormat simpledateformat = new SimpleDateFormat("dd MMM yyyy, HH:mm");
            String dateTime = simpledateformat.format(Calendar.getInstance().getTime());
            writer.append(dateTime + ": " + content + "\n\n");
            writer.flush();
            writer.close();
        }catch (Exception e){
            e.printStackTrace();

        }
    }

    public int mod(int value, int modValue)
    {
        int result = value % modValue;
        return result < 0? result + modValue : result;
    }

    // Set the status to Idle every 5 sec
    public void activateIdleCountDownTimer() {
        if(countDownTimer != null)
            countDownTimer.cancel();

        countDownTimer = new CountDownTimer(5000, 1000) {

            public void onTick(long millisUntilFinished) {
                // do nothing every sec until 5sec reach
            }

            public void onFinish() {
                if(ctrlFragment != null) {
                    ctrlFragment.setStatus(Protocol.STATUS_IDLE);
                }
            }
        }.start();
    }

    //Legacy code, this is for the stupid AMD tool

    private void handleGridUpdate(String gridData) {
        arenaview.getArena().updateGridMap(Operation.hexToBinary(gridData));
        ctrlFragment.setMDF1(gridData);
        if(ctrlFragment.getAutoStatus()) {
            arenaview.invalidate();
        }
    }


    private void handlePCGridUpdate(String part1, String part2) {
        arenaview.getArena().updateGridMapFromPc(Operation.hexToBinary(part1), Operation.hexToBinary(part2));
        ctrlFragment.setMDF1(arenaview.getArena().getMDF1());
        ctrlFragment.setMDF2(arenaview.getArena().getMDF2());
        if(ctrlFragment.getAutoStatus()) {
            arenaview.invalidate();
        }
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        long systemTime = System.currentTimeMillis();
        long time = systemTime - delayTime;
        if(time > 1000) {
            delayTime = systemTime;
            float x = event.values[0];
            float y = event.values[1];
            if (Math.abs(x) > Math.abs(y)) {
                if (x < 0) {
                    arenaview.getArena().getRobot().turnRight();
                    ctrlFragment.setStatus(Protocol.STATUS_TURN_RIGHT);
                    activateIdleCountDownTimer();
                    if(getConnectedDeviceName() != null && getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            getConnectedDeviceMac() != null && getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        BluetoothService.getInstance().sendText(Protocol.AMD_TURN_RIGHT.toString(), this);
                    }
                }
                if (x > 0) {
                    arenaview.getArena().getRobot().turnLeft();
                    ctrlFragment.setStatus(Protocol.STATUS_TURN_LEFT);
                    activateIdleCountDownTimer();
                    if(getConnectedDeviceName() != null && getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            getConnectedDeviceMac() != null && getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        BluetoothService.getInstance().sendText(Protocol.AMD_TURN_LEFT.toString(), this);
                    }
                }
            } else {
                if (y < 0) {
                    arenaview.getArena().getRobot().moveForward(true);
                    ctrlFragment.setStatus(Protocol.STATUS_FORWARD);
                    activateIdleCountDownTimer();
                    if(getConnectedDeviceName() != null && getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            getConnectedDeviceMac() != null && getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        BluetoothService.getInstance().sendText(Protocol.AMD_MOVE_FORWARD.toString(), this);
                    }
                }
                if (y > 0) {
                    arenaview.getArena().getRobot().reverse(true);
                    ctrlFragment.setStatus(Protocol.STATUS_REVERSE);
                    activateIdleCountDownTimer();
                    if(getConnectedDeviceName() != null && getConnectedDeviceName().equalsIgnoreCase(Protocol.AMD_DEVICE_NAME) &&
                            getConnectedDeviceMac() != null && getConnectedDeviceMac().equalsIgnoreCase(Protocol.AMD_DEVICE_MAC)) {
                        BluetoothService.getInstance().sendText(Protocol.AMD_REVERSE.toString(), this);
                    }
                }
            }
            if (x > (-2) && x < (2) && y > (-2) && y < (2)) {
                ctrlFragment.setStatus(Protocol.STATUS_IDLE);
            }
            arenaview.invalidate();
        }
    }
 //   BluetoothService.getInstance().sendText(Protocol.SEND_ARENA.toString(),context:)

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }

    public void activateTiltSensing(){
        senSensorManager.registerListener(this, senAccelerometer , SENSOR_DELAY_UI);
        delayTime = System.currentTimeMillis();
    }

    public void deactivateTiltSensing() {
        senSensorManager.unregisterListener(this);
    }
}