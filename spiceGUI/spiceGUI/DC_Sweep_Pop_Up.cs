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
    public partial class DC_Sweep_Pop_Up : Form
    {
        ListBox listbox;
        List<string> component;
        List<string> node;
        public DC_Sweep_Pop_Up(ListBox _listbox, List<string> _component, List<string> _node)
        {
            InitializeComponent();
            listbox = _listbox;
            component = _component;
            node = _node;
            comboBox1.DataSource = component;

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }

        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void label4_Click(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            string comp = comboBox1.GetItemText(comboBox1.SelectedItem);
            string VStart = textBox1.Text;
            string VStop = textBox2.Text;
            string Vincr = textBox3.Text;
            listbox.Items.Add("dc " + comp + "  " + VStart + " " + VStop + " " + Vincr);
        }
    }
}
