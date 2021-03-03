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
    public partial class Plot_Pop_Up : Form
    {
        ListBox listBox;
        List<string> Component;
        List<string> Node;
        string pltype;
        string prtype;
        string N1;
        string N2;
        public Plot_Pop_Up(ListBox _listBox, List<string> component, List<string> node)
        {
            InitializeComponent();
            listBox = _listBox;
            Component = component;
            Node = node;
            comboBox3.DataSource = node;
            comboBox4.DataSource = Node;

        }


        private void label4_Click(object sender, EventArgs e)
        {

        }

        private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
        {
            prtype = comboBox2.GetItemText(comboBox2.SelectedItem);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            listBox.Items.Add(".plot " + prtype + "(" + N1 + "," + N2 + ")");
        }

        private void comboBox3_SelectedIndexChanged(object sender, EventArgs e)
        {
            N1 = comboBox3.GetItemText(comboBox3.SelectedItem);
        }

        private void comboBox4_SelectedIndexChanged(object sender, EventArgs e)
        {
            N2 = comboBox4.GetItemText(comboBox4.SelectedItem);
        }
    }
}
