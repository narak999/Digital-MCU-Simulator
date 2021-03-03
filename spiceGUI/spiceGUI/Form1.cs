﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;

namespace spiceGUI
{
    public partial class Form1 : Form
    {
        List<string> component = new List<string>();
        List<string> node = new List<string>();
        public Form1()
        {
            InitializeComponent();
        }

        private void aCToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var AC_Small__Signal = new AC_Small__Signal_Pop_Up(listBox2);
            AC_Small__Signal.ShowDialog();
        }

        private void dCSweepToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var DC_Sweep_ = new DC_Sweep_Pop_Up(listBox2, component, node);
            DC_Sweep_.ShowDialog(); 
        }

        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {

        }

        private void plotToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            var plot = new Plot_Pop_Up(listBox2, component, node);
            plot.ShowDialog();
        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }



        //Generate
        private void button1_Click(object sender, EventArgs e)
        {
            string path = textBox3.Text;
            string pathOut = path.Substring(0,path.Length - 4) + "out.cir.txt";
            

            if (File.Exists(pathOut))
            {
                File.Delete(pathOut);
            }
            FileStream fsOut = File.Create(pathOut);
            byte[] b = new byte[1024];
                UTF8Encoding temp = new UTF8Encoding(true);
              //  string line;
            for(int i =0; i < listBox1.Items.Count; i++)
            {
                AddText(fsOut, listBox1.Items[i].ToString() + "\n");
            }

            AddText(fsOut, ".control\n");
            for(int i =0; i < listBox2.Items.Count; i++)
            {
                AddText(fsOut, listBox2.Items[i].ToString() + "\n");
            }
            AddText(fsOut, ".endc\n.end");
            fsOut.Close();

        }

        //Get Circuit
        private static void AddText(FileStream fs, string value)
        {
            byte[] info = new UTF8Encoding(true).GetBytes(value);
            fs.Write(info, 0, info.Length);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            //get path and source circuit 
            string path = textBox3.Text;
            System.IO.StreamReader fs = new System.IO.StreamReader(path);
            byte[] b = new byte[1024];
            UTF8Encoding temp = new UTF8Encoding(true);
            string line;

            //read line by line and add to list box
            while ((line = fs.ReadLine()).Length > 0)
            {
                listBox1.Items.Add(line);
            }
            generateComponentsList(listBox1);
            generateNodeList(listBox1);
        }
        
        private void generateComponentsList(ListBox listBox)
        {
            for(int i = 2; i<listBox.Items.Count; i++)
            {
                string comp = listBox.Items[i].ToString().Substring(0, listBox.Items[i].ToString().IndexOf(' '));
                if(!component.Contains(comp))
                    component.Add(comp);
            }
        }
        private void generateNodeList(ListBox listBox)
        {
            for (int i = 2; i < listBox.Items.Count; i++)
            {
                string firstword = listBox.Items[i].ToString().Substring(0, listBox.Items[i].ToString().IndexOf(' '));
                string remainingSentence1 = listBox.Items[i].ToString().Substring(listBox.Items[i].ToString().IndexOf(' ')+1);
               
                string secondword = remainingSentence1.Substring(0, remainingSentence1.IndexOf(' '));
                string remainingSentence2 = remainingSentence1.Substring(remainingSentence1.IndexOf(' ')+1);
                
                string thirdword = remainingSentence2.Substring(0, remainingSentence2.IndexOf(' '));

                if (!node.Contains(secondword))
                    node.Add(secondword);
                if (!node.Contains(thirdword))
                    node.Add(thirdword);
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void editToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void button4_Click(object sender, EventArgs e)
        {
            var EditSourceItem = new Edit_Source_Item_Pop_Up(listBox1, component, node);
            EditSourceItem.ShowDialog();
        }
        [System.Runtime.InteropServices.DllImport("user32.dll")]
        static extern IntPtr SetParent(IntPtr hWndChild, IntPtr hWndNewParent);

        private void button3_Click(object sender, EventArgs e)
        {
            OpenFileDialog od = new OpenFileDialog();
            if(od.ShowDialog() == DialogResult.OK)
            {
                Process proc = Process.Start(od.FileName);
                proc.WaitForInputIdle();

                while(proc.MainWindowHandle == IntPtr.Zero)
                {
                    Thread.Sleep(100);
                    proc.Refresh();
                }

                SetParent(proc.MainWindowHandle, this.Handle);
            }
        }
    }
    
}
