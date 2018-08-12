package com.example.artur_000.followmerobot;

import android.content.Context;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.widget.Button;
import android.widget.Toast;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.os.AsyncTask;
import java.io.IOException;
import java.util.UUID;

public class GpsControll extends AppCompatActivity {

    Button btnForw, btnDisc;
    String address = null;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    private boolean isBtConnect = false;
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    LocationManager locationManager;
    Location currentBestLocation;
    LocationListener locationListener;
    private static final int ONE_MINUTE = 1000 * 60 * 1;
    private static final int MY_PERMISSIONS_REQUEST_READ_CONTACTS = 3;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_gps_controll);
        locationManager = (LocationManager) this.getSystemService(Context.LOCATION_SERVICE);

        locationListener = new LocationListener() {
            @Override
            public void onLocationChanged(Location location) {
                // New Location found by the network location provider
                if (isBetterLocation(location, currentBestLocation)){
                    currentBestLocation = location;
                    msg("New Location");
                }
            }

            @Override
            public void onStatusChanged(String provider, int status, Bundle extras) {}

            @Override
            public void onProviderEnabled(String provider) {}

            @Override
            public void onProviderDisabled(String provider) {}
        };

        // Receive the address of the bluetooth device
        Intent newint = getIntent();
        address = newint.getStringExtra(DeviceList.EXTRA_ADDRESS);
        new ConnectBT().execute();

        // Register the listener with location manager to start requesting location data
        if (ContextCompat.checkSelfPermission(this, android.Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{android.Manifest.permission.ACCESS_FINE_LOCATION},
                    MY_PERMISSIONS_REQUEST_READ_CONTACTS);
        }else {
            initLocationRequest(locationListener);
        }

        // View of the layout
        setContentView(R.layout.activity_gps_controll);

        // Call the widgtes
        btnForw = (Button)findViewById(R.id.btnForward);
        btnForw.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Pass that location to sending method
                if (currentBestLocation != null) {
                    //msg("Latitude: " + currentBestLocation.getLatitude() + "\nLongitude: " + currentBestLocation.getLongitude());
                    send(currentBestLocation);
                }else{
                    initLocationRequest(locationListener);
                }
            }
        });

        btnDisc = (Button)findViewById(R.id.btn_disconnect);
        btnDisc.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                if (ContextCompat.checkSelfPermission(GpsControll.this, android.Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED) {
                    locationManager.removeUpdates(locationListener);
                }
                Disconnect();
            }
        });
    }

    private void initLocationRequest(LocationListener locationListener){
        if (ContextCompat.checkSelfPermission(GpsControll.this, android.Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED) {
            locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, 0, locationListener);
            locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 0, 0, locationListener);
            currentBestLocation = locationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == MY_PERMISSIONS_REQUEST_READ_CONTACTS){
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED){
                // Permission granted Phheewww
            }else{
                // Permission denied! Booo!!
                finish();
            }
        }
    }

    private boolean isBetterLocation(Location location, Location currBest){
        if (currBest == null){
            // If we don't have a current best location, then this is it!
            return true;
        }

        // Check time of location recieved
        long timeDela = location.getTime() - currBest.getTime();
        boolean isSignificantlyNewer = timeDela > ONE_MINUTE;
        boolean isSignificantlyOlder = timeDela < -ONE_MINUTE;
        boolean isNewer = timeDela > 0;

        if (isSignificantlyNewer){
            // Location is newer by 1 minute! User probs moved to use it!!
            return true;
        }else if (isSignificantlyOlder){
            // Eww the new location is stale don't use it !!
            return false;
        }

        // Check for the given accuracy if location
        int accuracyDelta = (int) (location.getAccuracy() - currBest.getAccuracy());
        boolean isMoreAccurate = accuracyDelta < 0;
        boolean isLessAccurate = accuracyDelta > 0;
        boolean isSignificantlyLessAccurate = accuracyDelta > 10;

        // Check if the old and new location are from the same provider
        boolean isFromSameProvider = isSameProvider(location.getProvider(), currentBestLocation.getProvider());

        // Determine location quality using a combination of timeliness and accuracy
        if (isMoreAccurate) {
            return true;
        } else if (isNewer && !isLessAccurate) {
            return true;
        } else if (isNewer && !isSignificantlyLessAccurate && isFromSameProvider) {
            return true;
        }
        return false;
    }

    /** Checks whether two providers are the same */
    private boolean isSameProvider(String provider1, String provider2) {
        if (provider1 == null) {
            return provider2 == null;
        }
        return provider1.equals(provider2);
    }

    private void send(Location location)
    {
        if (btSocket!=null)
        {
            try
            {
                msg(("<" + Double.toString(location.getLatitude()) + "," + Double.toString(location.getLongitude()) + ">"));
                btSocket.getOutputStream().write(("<" + Double.toString(location.getLatitude()) + "," + Double.toString(location.getLongitude()) + ">").getBytes());
            }
            catch (IOException e)
            {
                msg("Error");
            }
        }
    }

    private void Disconnect()
    {
        if (btSocket!=null) //If the btSocket is busy
        {
            try
            {
                btSocket.close(); //close connection
            }
            catch (IOException e)
            { msg("Error");}
        }
        finish(); //return to the first layout
    }

    private void msg(String s)
    {
        Toast.makeText(getApplicationContext(),s,Toast.LENGTH_LONG).show();
    }

    private class ConnectBT extends AsyncTask<Void, Void, Void> {

        private boolean connectSuccs = true;

        @Override
        protected void onPreExecute() {
            progress = ProgressDialog.show(GpsControll.this, "Connecting...", "Please wait!!!");  //show a progress dialog
        }

        @Override
        protected Void doInBackground(Void... params) { // Need to perform connection while the dialog is shown in the background
            try
            {
                if (btSocket == null || !isBtConnect)
                {
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();//get the mobile bluetooth device
                    BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);//connects to the device's address and checks if it's available
                    btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);//create a RFCOMM (SPP) connection
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();//start connection
                }
            }
            catch (IOException e)
            {
                connectSuccs = false;//if the try failed, you can check the exception here
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);
            if (!connectSuccs)
            {
                msg("Connection Failed. Is it on? Try again.");
                finish();
            }
            else
            {
                msg("Connected.");
                isBtConnect = true;
            }
            progress.dismiss();
        }
    }
}


