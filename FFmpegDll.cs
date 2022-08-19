using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using UnityEngine;
using AOT;

public class FFmpegDll
{
    //DLL Log Callback
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void LogDelegate(IntPtr message, int iSize);
    [DllImport("ffmpeg", CallingConvention = CallingConvention.Cdecl)]
    public static extern void InitCSharpDelegate(LogDelegate log);
    [MonoPInvokeCallback(typeof(LogDelegate))]
    public static void LogMessageFromCpp(IntPtr message, int iSize)
    {
        Debug.Log(Marshal.PtrToStringAnsi(message, iSize));
    }
    public static void ShowLog()
    {
        InitCSharpDelegate(LogMessageFromCpp);
    }

    [DllImport("ffmpeg")]
    public static extern void Mp4ToMp3(string srcFileName, string dstFileName,LogDelegate func);
    [DllImport("ffmpeg")]
    public static extern bool RecordStart(string dstFilePath,int width,int height,int fps);
    [DllImport("ffmpeg")]
    public static extern void WriteFrame(byte[] data,int length);
    [DllImport("ffmpeg")]
    public static extern int AddFrameToRecordQueue(byte[] dataPtr, int length);
    //[DllImport("FFmpegFrame")]
    //public static extern int FileWrite(byte[] dataPtr, int length);
    //[DllImport("FFmpegFrame")]
    //public static extern int ConsumedFrameNumber();
    [DllImport("ffmpeg")]
    public static extern int NumberOfFrameQueue();
    //[DllImport("FFmpegFrame")]
    //public static extern void ConsumedFrameSubSub();
    [DllImport("ffmpeg")]
    public static extern void RecordEnd();
    [DllImport("ffmpeg")]
    public static extern bool ExtractRGBStart(string srcFilePath, string dstFilePath, int width, int height, int fps);
}
