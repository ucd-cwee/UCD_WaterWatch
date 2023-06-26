/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using Excel = Microsoft.Office.Interop.Excel;
using Office = Microsoft.Office.Core;
using Microsoft.Office.Tools.Excel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using Foundation;
using System.Threading;

namespace Excel_WaterWatchLibrary
{
    /// <summary>
    /// Implementation of <see cref="INotifyPropertyChanged"/> to simplify models.
    /// </summary>
    public abstract class BindableBase : INotifyPropertyChanged
    {
        /// <summary>
        /// Multicast event for property change notifications.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Checks if a property already matches a desired value.  Sets the property and
        /// notifies listeners only when necessary.
        /// </summary>
        /// <typeparam name="T">Type of the property.</typeparam>
        /// <param name="storage">Reference to a property with both getter and setter.</param>
        /// <param name="value">Desired value for the property.</param>
        /// <param name="propertyName">Name of the property used to notify listeners.  This
        /// value is optional and can be provided automatically when invoked from compilers that
        /// support CallerMemberName.</param>
        /// <returns>True if the value was changed, false if the existing value matched the
        /// desired value.</returns>
        protected bool SetProperty<T>(ref T storage, T value, [CallerMemberName] String propertyName = null)
        {
            if (object.Equals(storage, value)) return false;

            storage = value;
            OnPropertyChanged(propertyName);
            return true;
        }

        /// <summary>
        /// Notifies listeners that a property value has changed.
        /// </summary>
        /// <param name="propertyName">Name of the property used to notify listeners.  This
        /// value is optional and can be provided automatically when invoked from compilers
        /// that support <see cref="CallerMemberNameAttribute"/>.</param>
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class WaterWatch_AppState : BindableBase
    {
        public WaterWatch_AppState(Excel.Application _app) {
            App = _app;
            udf_timer = new cweeTimer(1.0, () => {
                WaterWatchExcelMethods.EnsureUserDefinedFunctionsAreEnabled();
            }, false);
        }

        private cweeTimer udf_timer;

        private bool _UDF_Enabled = false;
        public bool UDF_Enabled
        {
            get
            {
                return _UDF_Enabled;
            }
            set
            {
                _UDF_Enabled = value;
                OnPropertyChanged("UDF_Enabled");
            }
        }
        public static Excel.Application App;
    }


    public partial class Excel_WaterWatchLibrary
    {
        public static Excel.Application App;
        public static WaterWatch_AppState state;
        public static System.Threading.Timer job_timer = new Timer(
                   new TimerCallback((object empty)=> {
                       Foundation.EdmsTasks.DoJob();
                   }),
                   null,
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor((1.0 / 60.0)), (int)(1000.0 * ((1.0 / 60.0) - Math.Floor((1.0 / 60.0))))
                   ),
                   new TimeSpan(
                       0, 0, 0, (int)Math.Floor((1.0 / 60.0)), (int)(1000.0 * ((1.0 / 60.0) - Math.Floor((1.0 / 60.0))))
                   )
            );

        private void Excel_WaterWatchLibrary_Startup(object sender, System.EventArgs e)
        {
            WaterWatch.SetDataDirectory(Application.StartupPath + @"\data"); // must happen immediately before CURL attempts to get our IP address 1 second after start-up.

            App = Application;
            state = new WaterWatch_AppState(App);
        }

        private void Excel_WaterWatchLibrary_Shutdown(object sender, System.EventArgs e)
        {

        }

        #region VSTO generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InternalStartup()
        {
            this.Startup += new System.EventHandler(Excel_WaterWatchLibrary_Startup);
            this.Shutdown += new System.EventHandler(Excel_WaterWatchLibrary_Shutdown);
        }
        
        #endregion
    }
}
