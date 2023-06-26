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
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using UWP_WaterWatch.Custom_Controls;
using UWP_WaterWatchLibrary;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=234238

namespace UWP_WaterWatch.Pages
{
    public class ScriptingPageViewModel : ViewModelBase
    {
        public static AtomicInt ObjCount = new AtomicInt();

        private static string Prelude()
        {
            string r = "";
            r = r.AddToDelimiter(
                "class ScriptNode{" +
                "   var uniqueID;" +
                "   var label;" +
                "   var script;" +
                "   var inputs;" +
                "   var version;" +
                "   var _result;" +
                "   var _inputVersion;" +
                "   var _evaluatedScript;" +
                "   var _working;" +
                "   def ScriptNode(){" +
                "       this.uniqueID = string(\"WW_Script\");" +
                "       this.label = string(\"New Script\");" +
                "       this.script = string();" +
                "       this.inputs = Map(); " +
                "       this.version = 0;" +
                "       this._result;" +
                "       this._working = bool(false);" +
                "       this._inputVersion = Map();" +
                "       this._evaluatedScript = string(\"\");" +
                "       this.set_explicit(true);" +
                "   };" +
                "};" +
                //"def `==`(ScriptNode a, ScriptNode b){ " +
                //"   return a.uniqueID == b.uniqueID; " +
                //"}; " +
                //"def `!=`(ScriptNode a, ScriptNode b){ " +
                //"   return a.uniqueID != b.uniqueID; " +
                //"}; " +
                "global __scriptNodeType = type(\"ScriptNode\"); " +
                "def _evaluate(ScriptNode node){" +
                "   __LOCK__{ if (node._working){ try { return node._result; } catch(e){ return; } }else{ node._working = true; } }" +
                "   do{ " +
                "       var funcHeader = \"string __script\";" +
                "       var& keys; __LOCK__{ " +
                "           keys := node.inputs.keys(); " +
                "       }; " +
                "       for (x : keys){" +
                "           if (x != \"\"){" +
                "              funcHeader = funcHeader.AddToDelimiter(x, \", \");" +
                "          }" +
                "       }" +
                "       var& FUNC; " +
                "       __LOCK__{ " +
                "           FUNC := eval(\"fun[](${funcHeader}){ eval(__script, \\\"${ node.uniqueID }\\\"); };\"); " +
                "       }; " +
                "       var Inputs = Vector();" +
                "       __LOCK__{ " +
                "           Inputs.push_back_ref(node.script); " +
                "       }; " +
                "       for (x : keys){ " +
                "           if (x == \"\") { continue; } " +
                "           var& k; " +
                "           __LOCK__{ " +
                "               k := node.inputs[x]; " +
                "           }; " +
                "           if (k.is_type(\"ScriptNode\")){ " +
                //"               if (k != node){ " +
                "                   Inputs.push_back_ref(k.result); " +
                //"               } else { " +
                //"                   __LOCK__{ " +
                //"                       Inputs.push_back_ref(k._result); " +
                //"                   }; " +
                //"               } " +
                "           } else { " +
                "               Inputs.push_back_ref(k); " +
                "           }; " +
                "       }; " +
                "       var& r := call(FUNC, Inputs); " +
                "       __LOCK__{ " +
                "           node._evaluatedScript = node.script; " +
                "           node.version++; " +
                "           node.remove_attr(\"_result\"); " +
                "           node.set_explicit(false); " +
                "           try { node._result := r; } catch(e){ node._result; } " +
                "           node.set_explicit(true); " +
                "           try { return node._result; } catch(e){ return; } " +
                "       }; " +
                "   }finally{ __LOCK__{ node._working = false; }; } " +
                "}; " +
                "def result(ScriptNode node, bool reevaluated){ " +
                "   __LOCK__{ if (node._working){ return node._result; } }; " +
                "   var reCalc = false; " +
                "   __LOCK__{" +
                "       if (node._evaluatedScript != node.script){ " +
                "           reCalc = true; " +
                "       } " +
                "   }; " +
                "   var& inputsCopy; " +
                "   __LOCK__{ " +
                "       inputsCopy := Map(node.inputs); " +
                "   }; " +
                "   for (x : inputsCopy) { " +
                "       if (x.second.is_type(\"ScriptNode\")){ " +
                "           bool wasRecalculated = false; " +
                "           x.second.result(wasRecalculated); " +
                "           if (wasRecalculated){ " +
                "               reCalc = true; " +
                "           } " +
                "       } " +
                "   } " +
                "   if (reCalc){ " +
                "       __LOCK__{ " +
                "           node._inputVersion = Map(); " +
                "       }; " +
                "   } " +
                "   for (x : inputsCopy) { " +
                "       __LOCK__{ " +
                "           if (x.second.is_type(\"ScriptNode\")){ " +
                "               if (node._inputVersion.contains(x.first)){ " +
                "                   if (node._inputVersion[x.first] != x.second.version){ " +
                "                       reCalc = true; " +
                "                  } " +
                "               } " +
                "               else { " +
                "                   reCalc = true; " +
                "              } " +
                "              node._inputVersion[x.first] = x.second.version; " +
                "           } " +
                "       }; " +
                "   } " +
                "   if (!reCalc){ " +
                "       reevaluated = false; " +
                "       __LOCK__{ " +
                "           return node._result; " +
                "       }; " +
                "   }else{ " +
                "       __LOCK__{ " +
                "           node._evaluatedScript = \"\"; " +
                "       }; " +
                "       reevaluated = true; " +
                "       if (node != null) { " +
                "           return _evaluate(node); " +
                "       } else { " +
                "           return; " +
                "       } " +
                "   } " +
                "}; " +
                "def result(ScriptNode node){ " +
                "   var temp = false; " +
                "   return result(node, temp); " +
                "}; " +
                "def invalidate(ScriptNode node){ " +
                "   __LOCK__{ " +
                "       node._evaluatedScript = \"\"; " +
                "   }; " +
                "}; "
            , "\n");

            return r;
        }

        private string _ScriptName = "New Script";
        public string ScriptName
        {
            get { return _ScriptName; }
            set
            {
                _ScriptName = value;
                OnPropertyChanged("ScriptName");
            }
        }

        private string _ScriptSavePath = "";
        public string ScriptSavePath
        {
            get { return _ScriptSavePath; }
            set
            {
                _ScriptSavePath = value;
                OnPropertyChanged("ScriptSavePath");
            }
        }

        public ScriptingPage_Workspace workspace;
        public ScriptEngine engine;
        public ScriptingManagerOutputPanel outputPanel;

        public ScriptingPageViewModel()
        {
            ObjCount.Increment();
            engine = new ScriptEngine();
            string preludeErr = engine.DoScript(Prelude());
            if (preludeErr != "") WaterWatch.SubmitToast("preludeErr", preludeErr);
        }
        ~ScriptingPageViewModel()
        {
            ObjCount.Decrement();
            workspace = null;
            outputPanel = null;
            engine = null;
        }
    }

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class ScriptingPage : Page
    {
        public static AtomicInt ObjCount = new AtomicInt();

        public event PropertyChangedEventHandler PropertyChanged;
        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            EdmsTasks.InsertJob(
                () => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName))
                , /*EdmsTasks.Priority.Low, */true, true
            );
        }

        public string ScriptName { get { return VM.ScriptName; } set { VM.ScriptName = value; OnPropertyChanged("ScriptName"); } }

        public ScriptingPageViewModel VM = new ScriptingPageViewModel(); // { get; set; }
        public ScriptingPage()
        {
            ObjCount.Increment();
            this.InitializeComponent();

            VM.outputPanel = OutputPanel;
            VM.workspace = Workspace;

            this.DataContext = VM;
        }
        ~ScriptingPage() {
            VM = null;
            ObjCount.Decrement();
        }
    }
}
