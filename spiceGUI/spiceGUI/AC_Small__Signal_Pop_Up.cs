using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace spiceGUI
{
    public partial class AC_Small__Signal_Pop_Up : Form
    {
        ListBox listBox;
        public AC_Small__Signal_Pop_Up(ListBox _ListBox)
        {
            listBox = _ListBox;
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string scale = comboBox1.GetItemText(comboBox1.SelectedItem);
            string numPoints = textBox1.Text;
            string fstart = textBox2.Text;
            string fstop = textBox3.Text;
            listBox.Items.Add("ac " + scale  + " " + numPoints + " " + fstart + " " + fstop);
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void AC_Small__Signal_Pop_Up_Load(object sender, EventArgs e)
        {

        }

        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }
    }
}
