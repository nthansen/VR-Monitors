using UnityEngine;
using System.Collections;
using System.Drawing;
using System.Windows.Forms;

public class ExternalForm : MonoBehaviour {

    Form current;
    TextBox viewSliderValue;
    //TextBox primaryMonitorSliderValue;
    TrackBar viewSlider;
    TrackBar primaryMonitorSlider;
    GameObject theCamera;
    GameObject primaryMonitor;
    //Vector3 primaryMonitorSize;

	// Use this for initialization
	void Start () {
        current = new System.Windows.Forms.Form();

        // get ahold of the camera to modify in the form
        theCamera = Camera.main.gameObject;

        // get the primary monitor in use
        primaryMonitor = GameObject.FindGameObjectWithTag("Primary Monitor");

      // primaryMonitorSize = primaryMonitor.transform.localScale;

        setUpForm();

        current.Show();
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    void setUpForm()
    {
        // removes minimize, exit, and full screen for form
        //current.ControlBox = false;

        Button quit = new Button();

        quit.Text = "Exit VR-Monitors";

        quit.Width = 100;

        quit.Location = new Point(5, 200);

        quit.Click += quit_Click;

        Label viewSliderLabel = new Label();

        viewSliderLabel.Text = "Camera distance";

        viewSliderLabel.Location = new Point(5, 10);

        viewSliderValue = new TextBox();

        viewSliderValue.Location = new Point(5, 80);

        viewSlider = new TrackBar();

        viewSlider.Location = new Point(5, 30);

        viewSlider.Scroll += new System.EventHandler(this.viewSlider_Scroll);

        viewSlider.Minimum = -100;

        // the max amount 
        viewSlider.Maximum = 100;

        // The TickFrequency property establishes how many positions 
        // are between each tick-mark.
        viewSlider.TickFrequency = 10;

        // The LargeChange property sets how many positions to move 
        // if the bar is clicked on either side of the slider.
        viewSlider.LargeChange = 3;

        // The SmallChange property sets how many positions to move 
        // if the keyboard arrows are used to move the slider.
        viewSlider.SmallChange = 2;


        // Used to create a slider for resizing the primary monitor, not neccessary for now
        /*
        Label primaryMonitorSliderLabel = new Label();

        primaryMonitorSliderLabel.Text = "Size of Primary Monitor";

        primaryMonitorSliderLabel.Location = new Point(5, 110);

        primaryMonitorSliderValue = new TextBox();

        primaryMonitorSliderValue.Location = new Point(5, 180);

        primaryMonitorSlider = new TrackBar();

        primaryMonitorSlider.Location = new Point(5, 130);

        primaryMonitorSlider.Scroll += new System.EventHandler(this.primaryMonitorSlider_Scroll);

        primaryMonitorSlider.Minimum = -100;

        // the max amount 
        primaryMonitorSlider.Maximum = 100;

        // The TickFrequency property establishes how many positions 
        // are between each tick-mark.
        primaryMonitorSlider.TickFrequency = 10;

        // The LargeChange property sets how many positions to move 
        // if the bar is clicked on either side of the slider.
        primaryMonitorSlider.LargeChange = 3;

        // The SmallChange property sets how many positions to move 
        // if the keyboard arrows are used to move the slider.
        primaryMonitorSlider.SmallChange = 2;
        */

        current.Controls.Add(quit);

        //current.Controls.Add(primaryMonitorSlider);

        //current.Controls.Add(primaryMonitorSliderValue);

        //current.Controls.Add(primaryMonitorSliderLabel);

        current.Controls.Add(viewSlider);

        current.Controls.Add(viewSliderValue);

        current.Controls.Add(viewSliderLabel);

        viewSliderValue.Text = "" + viewSlider.Value;

        //primaryMonitorSliderValue.Text = "" + primaryMonitorSlider.Value;

        current.Text = "I am testing";

        current.StartPosition = FormStartPosition.CenterScreen;
        
    }

    // what to do when the user scrolls on the viewSlider
    void viewSlider_Scroll(object sender, System.EventArgs e)
    {
        viewSliderValue.Text = "" + viewSlider.Value;

        // we need to move it back and forth so only change the z value
        Vector3 cameraPosition = new Vector3(theCamera.transform.position.x, theCamera.transform.position.y, viewSlider.Value);

        theCamera.transform.position = cameraPosition;
    }

    /*
    void primaryMonitorSlider_Scroll(object sender, System.EventArgs e)
    {
        primaryMonitorSliderValue.Text = "" + primaryMonitorSlider.Value;

        Vector3 primaryMonitorNewSize = new Vector3(primaryMonitorSize.x + primaryMonitorSlider.Value * 8, primaryMonitorSize.y + primaryMonitorSlider.Value * 5);


        // we need to move it back and forth so only change the z value
        primaryMonitor.transform.localScale = primaryMonitorNewSize;
    }
    */

      
    // for when a person clicks on the quit vr-monitors button on the form
    void quit_Click(object sender, System.EventArgs e)
    {
        current.Close();
        #if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
        #else 
            UnityEngine.Application.Quit();
        #endif
    }


    // make sure the form is closed with the application is closing
    void OnApplicationQuit()
    {
        current.Close();
    }
}
