using UnityEngine;
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

public class monitor : MonoBehaviour {

    Texture2D tex;
    Rectangle screenSize;
    Bitmap target;
    System.IO.MemoryStream ms;
    Vector3 scale;

	// Use this for initialization
	void Start () {
		screenSize = System.Windows.Forms.Screen.PrimaryScreen.Bounds;
        tex = new Texture2D(screenSize.Width, screenSize.Height, TextureFormat.RGB24, false);
		target = new Bitmap (screenSize.Width, screenSize.Height);
        scale = new Vector3(screenSize.Width, screenSize.Height, 0);
        transform.localScale += scale;
		using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(target)) {
			g.CopyFromScreen (0, 0, 0, 0, new Size (screenSize.Width, screenSize.Height));
		}
		ms = new System.IO.MemoryStream();
		target.Save (ms, ImageFormat.Png);
		ms.Seek (0, SeekOrigin.Begin);
		
		
		tex.LoadImage (ms.ToArray ());
		
		renderer.material.mainTexture = tex;
	}
	
	// Update is called once per frame
	void Update () {
        using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(target))
        {
            g.CopyFromScreen(0, 0, 0, 0, new Size(screenSize.Width, screenSize.Height));
        }
        target.Save(ms, ImageFormat.Png);
        ms.Seek(0, SeekOrigin.Begin);
        tex.LoadImage(ms.ToArray());

        renderer.material.mainTexture = tex;
	}
}
