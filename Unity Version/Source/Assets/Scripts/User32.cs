using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;

public class User32 : MonoBehaviour {

    public const Int32 CURSOR_SHOWING = 0x00000001;

    public struct ICONINFO
    {
        public bool fIcon;
        public Int32 xHotspot;
        public Int32 yHotspot;
        public IntPtr hbmMask;
        public IntPtr hbmColor;
    }

    public struct POINT
    {
        public Int32 x;
        public Int32 y;
    }

    public struct CURSORINFO
    {
        public Int32 cbSize;
        public Int32 flags;
        public IntPtr hCursor;
        public POINT ptScreenPos;
    }

    [DllImport("user32.dll")]
    public static extern bool GetCursorInfo(out CURSORINFO pci);

    [DllImport("user32.dll")]
    public static extern IntPtr CopyIcon(IntPtr hIcon);

    [DllImport("user32.dll")]
    public static extern bool DrawIcon(IntPtr hdc, int x, int y, IntPtr hIcon);

    [DllImport("user32.dll")]
    public static extern bool GetIconInfo(IntPtr hIcon, out ICONINFO piconinfo);
}
