

using UnityEngine;
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.IO;

public class monitor : MonoBehaviour {

    // streaming way once we figure out how to do it
    /*
    public WWW wwwData;
    public string url = "http://localhost:8080/desktop";
    private MovieTexture m;
    void Start()
    {
        wwwData = new WWW(url);
        m = wwwData.movie;
        renderer.material.shader = Shader.Find("Unlit/Texture");

    }
    void Update()
    {
        renderer.material.mainTexture = m as MovieTexture;
        // audio.clip = m.audioClip;
        if (!m.isPlaying)
        {
            m.Play();
            //audio.Play();
        }
    }
     */

    
    Texture2D tex;
    Rectangle screenSize;
    Bitmap target;
    System.IO.MemoryStream ms;
    Vector3 scale;

	// Use this for initialization
	void Awake () {

        UnityEngine.Application.runInBackground = true;

        screenSize = System.Windows.Forms.Screen.PrimaryScreen.Bounds;
        tex = new Texture2D(screenSize.Width, screenSize.Height, TextureFormat.ARGB32, false);
        target = new Bitmap(screenSize.Width, screenSize.Height, PixelFormat.Format24bppRgb);
        scale = new Vector3(screenSize.Width, screenSize.Height, 0);
        transform.localScale += scale;
        target = Capture(screenSize.X, screenSize.Y, screenSize.Width, screenSize.Height, target);
        
		ms = new System.IO.MemoryStream(1024);
		target.Save (ms, ImageFormat.Png);

        // to test the quality of the screen grab. 
        //target.Save("f:\\desktopTest.png", ImageFormat.Png);
		ms.Seek (0, SeekOrigin.Begin);
		
		tex.LoadImage (ms.ToArray ());
		
		renderer.material.mainTexture = tex;

        renderer.material.mainTexture.anisoLevel = 9;

        renderer.material.shader = Shader.Find("Unlit/Texture");
	}
	
	// Update is called once per frame
    void Update()
    {

        target = Capture(screenSize.X, screenSize.Y, screenSize.Width, screenSize.Height, target);
        using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(target))
            target.Save(ms, ImageFormat.Png);
        ms.Seek(0, SeekOrigin.Begin);
        tex.LoadImage(ms.ToArray());
        renderer.material.mainTexture = tex;
    }

    Bitmap Capture(int x, int y, int width, int height, Bitmap target)
    {
        using (System.Drawing.Graphics g = System.Drawing.Graphics.FromImage(target))
        {
            g.CopyFromScreen(x, y, 0, 0, target.Size, CopyPixelOperation.SourceCopy);

            User32.CURSORINFO cursorInfo;
            cursorInfo.cbSize = Marshal.SizeOf(typeof(User32.CURSORINFO));

            if (User32.GetCursorInfo(out cursorInfo))
            {
                // if the cursor is showing draw it on the screen shot
                if (cursorInfo.flags == User32.CURSOR_SHOWING)
                {
                    // we need to get hotspot so we can draw the cursor in the correct position
                    var iconPointer = User32.CopyIcon(cursorInfo.hCursor);
                    User32.ICONINFO iconInfo;
                    int iconX, iconY;

                    if (User32.GetIconInfo(iconPointer, out iconInfo))
                    {
                        // calculate the correct position of the cursor
                        iconX = cursorInfo.ptScreenPos.x - ((int)iconInfo.xHotspot);
                        iconY = cursorInfo.ptScreenPos.y - ((int)iconInfo.yHotspot);

                        // calculate offset of primary monitor
                        iconX -= x;
                        iconY -= y;

                        // draw the cursor icon on top of the captured screen image
                        User32.DrawIcon(g.GetHdc(), iconX, iconY, cursorInfo.hCursor);

                        // release the handle created by call to g.GetHdc()
                        g.ReleaseHdc();
                    }
                }
            }
        }
        return target;
    }
    
}
