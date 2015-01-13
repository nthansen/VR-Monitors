using UnityEngine;
using System.Collections;
using System.Drawing;
using System.Windows.Forms;

public class ExternalForm : MonoBehaviour {

    Form current;
    TextBox viewSliderValue;
    TrackBar viewSlider;
    GameObject theCamera;

	// Use this for initialization
	void Start () {
        current = new System.Windows.Forms.Form();

        // get ahold of the camera to modify in the form
        theCamera = Camera.main.gameObject;

        setUpForm();

        current.Show();
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    void setUpForm()
    {
        Button button1 = new Button();

        button1.Text = "This is a test";

        Label viewSliderLabel = new Label();

        viewSliderLabel.Text = "Camera distance";

        viewSliderLabel.Location = new Point(5, 130);

        viewSliderValue = new TextBox();

        viewSliderValue.Location = new Point(5, 200);

        viewSlider = new TrackBar();

        viewSlider.Location = new Point(5, 150);

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

        current.Controls.Add(button1);

        current.Controls.Add(viewSlider);

        current.Controls.Add(viewSliderValue);

        current.Controls.Add(viewSliderLabel);

        viewSliderValue.Text = "" + viewSlider.Value;

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
}
