using UnityEngine;
using System.Collections;
using System.Drawing;
using System.Windows.Forms;

public class ExternalForm : MonoBehaviour {

    Form current;
    CheckBox viewCrosshair;
    CheckBox moveMonitor;
    ComboBox chooseBackground;
    TrackBar cameraDistanceSlider;
    TrackBar cameraHeightSlider;
    public TrackBar resizeMonitorSlider;
    public TrackBar rotateMonitorSlider;
    GameObject theCamera;
    GameObject crosshair;
    public static ExternalForm instance;

	// Use this for initialization
	void Start () {
        current = new System.Windows.Forms.Form();

        // get ahold of the camera to modify in the form
        theCamera = Camera.main.gameObject;

        crosshair = GameObject.FindGameObjectWithTag("Crosshair");

        setUpForm();

        current.Show();

        instance = this;
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    void setUpForm()
    {
        // removes minimize, exit, and full screen for form
        //current.ControlBox = false;

        createCameraFunctions();

        createMonitorFunctions();

        Button quit = new Button();

        quit.Text = "Exit VR-Monitors";

        quit.Width = 100;

        quit.Location = new Point(5, 230);

        quit.Click += quit_Click;


        viewCrosshair = new CheckBox();

        viewCrosshair.Text = "View Crosshair";

        viewCrosshair.Click += new System.EventHandler(this.viewCrosshair_Click);

        viewCrosshair.Location = new Point(160, 80);

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

        current.Text = "VR-Monitors Control Panel";

        current.StartPosition = FormStartPosition.CenterScreen;
        
    }

    void createMonitorFunctions() {

        createAddMonitor();

        createMoveMonitor();

        createResizeMonitor();

        createRotateMonitor();

        createResetMonitors();
    }

    void createAddMonitor()
    {
        Button addMonitor = new Button();

        addMonitor.Text = "Add Monitor";

        addMonitor.Location = new Point(160, 40);

        addMonitor.Click += addMonitor_Click;

        current.Controls.Add(addMonitor);
    }

    void createMoveMonitor()
    {
        moveMonitor = new CheckBox();

        moveMonitor.Appearance = Appearance.Button;

        moveMonitor.Text = "Move Monitor";

        moveMonitor.Click += new System.EventHandler(this.moveMonitor_Click);

        moveMonitor.Location = new Point(5, 150);

        current.Controls.Add(moveMonitor);
    }

    void createResizeMonitor()
    {
        Label resizeMonitorLabel = new Label();

        resizeMonitorLabel.Text = "Resize Monitor";

        resizeMonitorLabel.Location = new Point(160, 120);

        current.Controls.Add(resizeMonitorLabel);
        
        resizeMonitorSlider = new TrackBar();

        resizeMonitorSlider.Location = new Point(160, 150);

        resizeMonitorSlider.Scroll += new System.EventHandler(this.resizeMonitorSlider_Scroll);

        resizeMonitorSlider.Minimum = -4;

        // the max amount 
        resizeMonitorSlider.Maximum = 8;

        // The TickFrequency property establishes how many positions 
        // are between each tick-mark.
        resizeMonitorSlider.TickFrequency = 4;

        // The LargeChange property sets how many positions to move 
        // if the bar is clicked on either side of the slider.
        resizeMonitorSlider.LargeChange = 2;

        // The SmallChange property sets how many positions to move 
        // if the keyboard arrows are used to move the slider.
        resizeMonitorSlider.SmallChange = 1;

        current.Controls.Add(resizeMonitorSlider);
        
    }

    void resizeMonitorSlider_Scroll(object sender, System.EventArgs e)
    {
        GameObject resizedMonitor = Reticle.instance.selectedMonitor;

        if (resizedMonitor != null)
        {
            monitor values = (monitor)resizedMonitor.GetComponent("monitor");

            values.monitorSize = resizeMonitorSlider.Value;

            Vector3 resizedMonitorNewSize = new Vector3(values.originalScale.x + resizeMonitorSlider.Value, values.originalScale.y + resizeMonitorSlider.Value);

            resizedMonitor.transform.localScale = resizedMonitorNewSize;
        }
    }

    void createRotateMonitor()
    {
        Label rotateMonitorLabel = new Label();

        rotateMonitorLabel.Text = "Rotate Monitor";

        rotateMonitorLabel.Location = new Point(160, 200);

        current.Controls.Add(rotateMonitorLabel);

        rotateMonitorSlider = new TrackBar();

        rotateMonitorSlider.Location = new Point(160, 220);

        rotateMonitorSlider.Scroll += new System.EventHandler(this.rotateMonitorSlider_Scroll);

        rotateMonitorSlider.Minimum = -8;

        // the max amount 
        rotateMonitorSlider.Maximum = 8;

        // The TickFrequency property establishes how many positions 
        // are between each tick-mark.
        rotateMonitorSlider.TickFrequency = 4;

        // The LargeChange property sets how many positions to move 
        // if the bar is clicked on either side of the slider.
        rotateMonitorSlider.LargeChange = 2;

        // The SmallChange property sets how many positions to move 
        // if the keyboard arrows are used to move the slider.
        rotateMonitorSlider.SmallChange = 1;

        current.Controls.Add(rotateMonitorSlider);
    }

    void rotateMonitorSlider_Scroll(object sender, System.EventArgs e)
    {
        GameObject rotatedMonitor = Reticle.instance.selectedMonitor;

        if (rotatedMonitor != null)
        {
            monitor values = (monitor)rotatedMonitor.GetComponent("monitor");

            values.monitorRotation = rotateMonitorSlider.Value;

            Vector3 rotatedMonitorNewSpot = new Vector3(rotatedMonitor.transform.localEulerAngles.x, 
                values.originalRotation.y + rotateMonitorSlider.Value * 4, rotatedMonitor.transform.localEulerAngles.z);

            rotatedMonitor.transform.localEulerAngles = rotatedMonitorNewSpot;
        }
    }

    void createResetMonitors()
    {
        Button reset = new Button();

        reset.Text = "Reset Monitor Positions";

        reset.Width = 150;

        reset.Location = new Point(5, 180);

        reset.Click += resetMonitors_Click;

        current.Controls.Add(reset);
    }

    void resetMonitors_Click(object sender, System.EventArgs e)
    {
        MonitorController.instance.resetMonitors();
    }

    void createCameraFunctions()
    {
        createCameraDistance();

        createCameraHeight();

        createCameraReset();
    }

    void createCameraDistance()
    {
        Label cameraDistanceSliderLabel = new Label();

        cameraDistanceSliderLabel.Text = "Camera distance";

        cameraDistanceSliderLabel.Location = new Point(5, 10);

        cameraDistanceSlider = new TrackBar();

        cameraDistanceSlider.Location = new Point(5, 30);

        cameraDistanceSlider.Scroll += new System.EventHandler(this.cameraDistanceSlider_Scroll);

        cameraDistanceSlider.Minimum = -10;

        // the max amount 
        cameraDistanceSlider.Maximum = 10;

        // The TickFrequency property establishes how many positions 
        // are between each tick-mark.
        cameraDistanceSlider.TickFrequency = 5;

        // The LargeChange property sets how many positions to move 
        // if the bar is clicked on either side of the slider.
        cameraDistanceSlider.LargeChange = 2;

        // The SmallChange property sets how many positions to move 
        // if the keyboard arrows are used to move the slider.
        cameraDistanceSlider.SmallChange = 1;

        current.Controls.Add(cameraDistanceSlider);

        current.Controls.Add(cameraDistanceSliderLabel);
    }

    // what to do when the user scrolls on the viewSlider
    void cameraDistanceSlider_Scroll(object sender, System.EventArgs e)
    {
        // we need to move it back and forth so only change the z value
        Vector3 cameraPosition = new Vector3(theCamera.transform.position.x, theCamera.transform.position.y, cameraDistanceSlider.Value);

        theCamera.transform.position = cameraPosition;
    }

    void createCameraHeight()
    {
        Label cameraHeightSliderLabel = new Label();

        cameraHeightSliderLabel.Text = "Camera Height";

        cameraHeightSliderLabel.Location = new Point(5, 80);

        cameraHeightSlider = new TrackBar();

        cameraHeightSlider.Location = new Point(5, 100);

        cameraHeightSlider.Scroll += new System.EventHandler(this.cameraHeightSlider_Scroll);

        // the max amount 
        cameraHeightSlider.Maximum = 10;

        // The TickFrequency property establishes how many positions 
        // are between each tick-mark.
        cameraHeightSlider.TickFrequency = 2;

        // The LargeChange property sets how many positions to move 
        // if the bar is clicked on either side of the slider.
        cameraHeightSlider.LargeChange = 2;

        // The SmallChange property sets how many positions to move 
        // if the keyboard arrows are used to move the slider.
        cameraHeightSlider.SmallChange = 1;

        current.Controls.Add(cameraHeightSlider);

        current.Controls.Add(cameraHeightSliderLabel);
    }

    void cameraHeightSlider_Scroll(object sender, System.EventArgs e)
    {
        // we need to move it back and forth so only change the z value
        Vector3 cameraPosition = new Vector3(theCamera.transform.position.x, cameraHeightSlider.Value, theCamera.transform.position.z);

        theCamera.transform.position = cameraPosition;
    }

    void createCameraReset()
    {
        Button reset = new Button();

        reset.Text = "Recenter Camera";

        reset.Width = 150;

        reset.Location = new Point(5, 205);

        reset.Click += cameraReset_Click;

        current.Controls.Add(reset);
    }

    void cameraReset_Click(object sender, System.EventArgs e)
    {
        OVRManager.display.RecenterPose();
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

    void addMonitor_Click(object sender, System.EventArgs e)
    {
        MonitorController.instance.addMonitor = true;
    }

    // make sure the form is closed with the application is closing
    void OnApplicationQuit()
    {
        current.Close();
    }
}
