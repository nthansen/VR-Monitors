using UnityEngine;
using System.Collections;
using System.Drawing;
using System.Windows.Forms;

public class ExternalForm : MonoBehaviour {

    Form current;

	// Use this for initialization
	void Start () {
        current = new System.Windows.Forms.Form();
        Button button1 = new Button();
        button1.Text = "This is a test";
        current.Controls.Add(button1);
        current.Text = "I am testing";
        current.StartPosition = FormStartPosition.CenterScreen;
        current.Show();
    }
	
	// Update is called once per frame
	void Update () {
	
	}
}
