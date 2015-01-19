using UnityEngine;
using System.Collections;
using System.Drawing;
using System.Windows.Forms;

public class ExternalForm : MonoBehaviour {

    Form current;
    TextBox viewSliderValue;
    CheckBox viewCrosshair;
    CheckBox moveMonitor;
    ComboBox chooseBackground;
    //TextBox primaryMonitorSliderValue;
    TrackBar viewSlider;
    TrackBar primaryMonitorSlider;
    GameObject theCamera;
    GameObject primaryMonitor;
    GameObject crosshair;
    //Vector3 primaryMonitorSize;

	// Use this for initialization
	void Start () {
        current = new System.Windows.Forms.Form();

        // get ahold of the camera to modify in the form
        theCamera = Camera.main.gameObject;

        // get the primary monitor in use
        primaryMonitor = GameObject.FindGameObjectWithTag("Primary Monitor");

        // primaryMonitorSize = primaryMonitor.transform.localScale;

        crosshair = GameObject.FindGameObjectWithTag("Crosshair");

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

        viewCrosshair = new CheckBox();

        viewCrosshair.Text = "View Crosshair";

        viewCrosshair.Click += new System.EventHandler(this.viewCrosshair_Click);

        viewCrosshair.Location = new Point(5, 120);

        moveMonitor = new CheckBox();

        moveMonitor.Appearance = Appearance.Button;

        moveMonitor.Text = "Move Monitor";

        moveMonitor.Click += new System.EventHandler(this.moveMonitor_Click);
        
        moveMonitor.Location = new Point(5, 150);

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

        chooseBackground = new ComboBox();

        chooseBackground.Location = new Point(160, 10);

        chooseBackground.Name = "choose Background";

        chooseBackground.Text = "Eerie";

        string[] backgrounds = new string[] { "Dawn Dusk", "Eerie", "Moon Shine", "Overcast1", "Overcast2", "Starry Night", "Sunny1", "Sunny2", "Sunny3" };

        chooseBackground.Items.AddRange(backgrounds);

        chooseBackground.DropDownClosed += new System.EventHandler(chooseBackground_DropDown);

        current.Controls.Add(chooseBackground);

        current.Controls.Add(quit);

        current.Controls.Add(viewCrosshair);

        current.Controls.Add(moveMonitor);

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
        Vector3 cameraPosition = new Vector3(theCamera.transform.position.x, theCamera.transform.position.y, viewSlider.Value * 5);

        theCamera.transform.position = cameraPosition;
    }

    void viewCrosshair_Click(object sender, System.EventArgs e)
    {
        if (viewCrosshair.Checked)
        {
            crosshair.renderer.enabled = true;
        }
        else
        {
            crosshair.renderer.enabled = false;
        }
    }

    void moveMonitor_Click(object sender, System.EventArgs e)
    {
        if (moveMonitor.Checked)
        {
            if (Reticle.instance.selectedMonitor != null)
            {
                moveMonitor.Text = "Place Monitor";
                Reticle.instance.moveTheMonitor = true;
            }
            else
            {
                moveMonitor.Checked = false;
            }
        }
        else
        {
            moveMonitor.Text = "Move Monitor";
            Reticle.instance.moveTheMonitor = false;
        }

    }

    void chooseBackground_DropDown(object sender, System.EventArgs e)
    {
        // reference list of the type of backgrounds we have "Dawn Dusk", 
        //"Eerie", "Moon Shine", "Overcast1", "Overcast2", "Starry Night", "Sunny1", "Sunny2", "Sunny3"
 
        if (chooseBackground.Text == "Dawn Dusk")
        {
            RenderSettings.skybox = (Material)Resources.Load("Skyboxes/DawnDusk Skybox", typeof(Material));
        }

        else if (chooseBackground.Text == "Eerie")
        {
            RenderSettings.skybox = (Material)Resources.Load("Skyboxes/Eerie Skybox", typeof(Material));
        }
        else if (chooseBackground.Text == "Moon Shine")
        {
            RenderSettings.skybox = (Material)Resources.Load("Skyboxes/MoonShine Skybox", typeof(Material));
        }
        else if (chooseBackground.Text == "Overcast1")
        {
            RenderSettings.skybox = (Material)Resources.Load("Skyboxes/Overcast1 Skybox", typeof(Material));
        }
        else if (chooseBackground.Text == "Overcast2")
        {
            RenderSettings.skybox = (Material)Resources.Load("Skyboxes/Overcast2 Skybox", typeof(Material));
        }
        else if (chooseBackground.Text == "Starry Night")
        {
            RenderSettings.skybox = (Material)Resources.Load("Skyboxes/StarryNight Skybox", typeof(Material));
        }
        else if (chooseBackground.Text == "Sunny1")
        {
            RenderSettings.skybox = (Material)Resources.Load("Skyboxes/Sunny1 Skybox", typeof(Material));
        }
        else if (chooseBackground.Text == "Sunny2")
        {
            RenderSettings.skybox = (Material)Resources.Load("Skyboxes/Sunny2 Skybox", typeof(Material));
        }
        else if (chooseBackground.Text == "Sunny3")
        {
            RenderSettings.skybox = (Material)Resources.Load("Skyboxes/Sunny3 Skybox", typeof(Material));
        }
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
