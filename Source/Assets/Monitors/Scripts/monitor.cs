using UnityEngine;
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

public class monitor : MonoBehaviour {

	// Use this for initialization
	void Start () {
		Texture2D tex = new Texture2D (200, 300, TextureFormat.RGB24, false);
		Rectangle screenSize = System.Windows.Forms.Screen.PrimaryScreen.Bounds;
		Bitmap target = new Bitmap (screenSize.Width, screenSize.Height);
		using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(target)) {
			g.CopyFromScreen (0, 0, 0, 0, new Size (screenSize.Width, screenSize.Height));
		}
		System.IO.MemoryStream ms = new System.IO.MemoryStream();
		target.Save (ms, ImageFormat.Png);
		ms.Seek (0, SeekOrigin.Begin);
		
		
		tex.LoadImage (ms.ToArray ());
		
		renderer.material.mainTexture = tex;
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
