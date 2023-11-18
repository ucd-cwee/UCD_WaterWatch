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
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using UWP_WaterWatchLibrary;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.System;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Text;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using UWP_WaterWatch;
using static UWP_WaterWatchLibrary.ObjectExtensions;
using System.Collections.Concurrent;
using Microsoft.UI.Xaml.Controls;
using MicaEditor;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace UWP_WaterWatch.Custom_Controls
{
    public class CodeError : ViewModelBase
    {
        public string Error;
        public int ErrorCode;
        // public string ParentNodeUniqueID;
    }
    public class NodeErrorManager : ViewModelBase
    {
        private static Dictionary<int, string> CodeToWarning = new Dictionary<int, string>() {
              { 0, "No Error" }
            , { 1, "No Error" }
            , { 2, "No Error" }
        };
        public System.Collections.Concurrent.ConcurrentDictionary<int, CodeError> Errors = new ConcurrentDictionary<int, CodeError>();
        public int NumWarnings => CodeToWarning.Count;
        public void AddWarning(int code, string warning, ref ScriptingNodeViewModel ParentVM) {
            RemoveWarning(code);
            CodeToWarning[code] = warning;
            Errors[code] = new CodeError() { Error = GetWarningFromCode(code), ErrorCode = code };

            ParentVM.outputPanel.vm.StatusString = $"Node '{ParentVM.Label}': '{GetWarningFromCode(code)}'";

            OnPropertyChanged("Errors"); OnPropertyChanged("NumWarnings");
        }
        public void AddWarning(int code) {
            RemoveWarning(code);
            Errors[code] = new CodeError() { Error = GetWarningFromCode(code), ErrorCode = code };
            OnPropertyChanged("Errors"); OnPropertyChanged("NumWarnings");
        }
        public void RemoveWarning(int code) {
            Errors.TryRemove(code, out CodeError v);
            OnPropertyChanged("Errors"); OnPropertyChanged("NumWarnings");
        }
        public string GetWarning(int code)
        {
            if (Errors.TryGetValue(code, out CodeError v)) {
                return v.Error;
            }
            return null;
        }
        public static string GetWarningFromCode(int code)
        {
            if (CodeToWarning.TryGetValue(code, out string v))
            {
                return v;
            }
            else
            {
                return "";
            }
        }
    }

    public class ScriptNode_EditorVM : ViewModelBase
    {
        public static AtomicInt ObjCount = new AtomicInt();

        public static Point pointerPosition = new Point(0,0);
        public static AtomicInt pointerPositionLock = new AtomicInt();
        public static cweeTimer pointerTracker;

        public NodeErrorManager errorManager;
        public ScriptingNodeViewModel ParentVM = null;
        public cweeTimer hoverProcessor;

        public ScriptNode_EditorVM()
        {
            ObjCount.Increment();
            errorManager = new NodeErrorManager();
        }
        ~ScriptNode_EditorVM() {
            if (ObjCount.Decrement() == 0) {
                // pointerTracker = null;
            }
            ParentVM = null;
            pointerTracker = null;
            hoverProcessor = null;
            errorManager = null;
        }

        public vector_scriptingnode MostRecentParsedNodes_NoError;
        public string MostRecentParsedScript_NoError;

        public vector_scriptingnode MostRecentParsedNodes;
    }

#if true
    public class RichEditBoxExtension
    {
        // Standard attached property. It mimics the "Text" property of normal text boxes
        public static readonly DependencyProperty PlainTextProperty =
          DependencyProperty.RegisterAttached("PlainText", typeof(string),
          typeof(RichEditBoxExtension), new PropertyMetadata(null, OnPlainTextChanged));

        // Standard DP infrastructure
        public static string GetPlainText(DependencyObject o)
        {
            return (o.GetValue(PlainTextProperty) as string).Replace("\r", "\n");
        }

        // Standard DP infrastructure
        public static void SetPlainText(DependencyObject o, string s)
        {
            o.SetValue(PlainTextProperty, s == null ? s : s.Replace("\r", "\n"));
        }

        private static void OnPlainTextChanged(DependencyObject o,
          DependencyPropertyChangedEventArgs e)
        {
            var source = o as RichEditBox;
            if (o == null || e.NewValue == null)
                return;

            // This attaches an event handler for the TextChange event in the RichEditBox,
            // ensuring that we're made aware of any changes
            AttachRichEditBoxChangingHelper(o);

            // To avoid endless property updates, we make sure we only change the RichText's 
            // Document if the PlainText was modified (vs. if PlainText is responding to 
            // Document being modified)
            var state = GetState(o);
            switch (state)
            {
                case RichEditChangeState.Idle:
                    var text = (e.NewValue as string).Replace("\r", "\n");
                    SetState(o, RichEditChangeState.PlainTextChanged);
                    source.TextDocument.SetText(Windows.UI.Text.TextSetOptions.None, text);
                    var paragraphFormat = source.TextDocument.GetDefaultParagraphFormat();
                    paragraphFormat.PageBreakBefore = FormatEffect.Off;
                    paragraphFormat.Style = ParagraphStyle.None;
                    paragraphFormat.WidowControl = FormatEffect.Off;
                    paragraphFormat.SetLineSpacing(LineSpacingRule.Single, 0);
                    source.TextDocument.SetDefaultParagraphFormat(paragraphFormat);
                    // source.FontSize = 12;
                    source.FontFamily = new FontFamily("Segoe UI");
                    source.FontStretch = FontStretch.Normal;
                    source.ClipboardCopyFormat = RichEditClipboardFormat.PlainText;
                    source.HorizontalTextAlignment = TextAlignment.Left;
                    break;

                case RichEditChangeState.RichTextChanged:
                    SetState(o, RichEditChangeState.Idle);
                    break;

                default:
                    SetState(o, RichEditChangeState.Idle);
                    break;
            }
        }

#region Glue

        // Trivial state machine to determine who last changed the text properties
        enum RichEditChangeState
        {
            Idle,
            RichTextChanged,
            PlainTextChanged,
            Unknown
        }

        // Helper class that just stores a state inside a textbox, determining
        // whether it is already being changed by code or not
        class RichEditChangeStateHelper
        {
            public RichEditChangeState State { get; set; }
        }

        // Private attached property (never seen in XAML or anywhere else) to attach
        // the state variable for us. Because this isn't used in XAML, we don't need
        // the normal GetXXX and SetXXX static methods.
        static readonly DependencyProperty RichEditChangeStateHelperProperty =
          DependencyProperty.RegisterAttached("RichEditChangeStateHelper",
          typeof(RichEditChangeStateHelper), typeof(RichEditBoxExtension), null);

        // Inject our state into the textbox, and also attach an event-handler
        // for the TextChanged event.
        static void AttachRichEditBoxChangingHelper(DependencyObject o)
        {
            if (o.GetValue(RichEditChangeStateHelperProperty) != null)
                return;

            var richEdit = o as RichEditBox;
            var helper = new RichEditChangeStateHelper();
            o.SetValue(RichEditChangeStateHelperProperty, helper);

            richEdit.TextChanged += (sender, args) =>
            {
                // To avoid re-entrancy, make sure we're not already changing
                var state = GetState(o);
                switch (state) {
                    case RichEditChangeState.Idle:
                        string text = null;
                        richEdit.TextDocument.GetText(Windows.UI.Text.TextGetOptions.NoHidden, out text);
                        if (text != GetPlainText(o))
                        {
                            SetState(o, RichEditChangeState.RichTextChanged);
                            o.SetValue(PlainTextProperty, text.Replace("\r", "\n"));
                        }
                        break;
                    case RichEditChangeState.PlainTextChanged:
                        SetState(o, RichEditChangeState.Idle);
                        break;
                    default:
                        SetState(o, RichEditChangeState.Idle);
                        break;
                }
            };
        }

        // Helper to set the state managed by the textbox
        static void SetState(DependencyObject o, RichEditChangeState state)
        {
            (o.GetValue(RichEditChangeStateHelperProperty) as RichEditChangeStateHelper).State = state;
        }

        // Helper to get the state managed by the textbox
        static RichEditChangeState GetState(DependencyObject o)
        {
            return (o.GetValue(RichEditChangeStateHelperProperty) as RichEditChangeStateHelper).State;
        }
#endregion
    }

    public class CodeEditorControlExtension
    {
        // Standard attached property. It mimics the "Text" property of normal text boxes
        public static readonly DependencyProperty PlainTextProperty =
          DependencyProperty.RegisterAttached("PlainText", typeof(string),
          typeof(CodeEditorControlExtension), new PropertyMetadata(null, OnPlainTextChanged));

        // Standard DP infrastructure
        public static string GetPlainText(DependencyObject o)
        {
            return (o.GetValue(PlainTextProperty) as string).Replace("\r", "\n");
        }

        // Standard DP infrastructure
        public static void SetPlainText(DependencyObject o, string s)
        {
            o.SetValue(PlainTextProperty, s == null ? s : s.Replace("\r", "\n"));
        }

        private static void OnPlainTextChanged(DependencyObject o,
          DependencyPropertyChangedEventArgs e)
        {
            var source = o as CodeEditorControl;
            if (o == null || e.NewValue == null)
                return;

            // This attaches an event handler for the TextChange event in the CodeEditorControl,
            // ensuring that we're made aware of any changes
            AttachCodeEditorControlChangingHelper(o);

            // To avoid endless property updates, we make sure we only change the RichText's 
            // Document if the PlainText was modified (vs. if PlainText is responding to 
            // Document being modified)
            var state = GetState(o);
            switch (state)
            {
                case RichEditChangeState.Idle:
                    var text = (e.NewValue as string).Replace("\r", "\n");
                    SetState(o, RichEditChangeState.PlainTextChanged);

                    source.Editor.SetText(text);
                    break;

                case RichEditChangeState.RichTextChanged:
                    SetState(o, RichEditChangeState.Idle);
                    break;

                default:
                    SetState(o, RichEditChangeState.Idle);
                    break;
            }
        }

        #region Glue

        // Trivial state machine to determine who last changed the text properties
        enum RichEditChangeState
        {
            Idle,
            RichTextChanged,
            PlainTextChanged,
            Unknown
        }

        // Helper class that just stores a state inside a textbox, determining
        // whether it is already being changed by code or not
        class RichEditChangeStateHelper
        {
            public RichEditChangeState State { get; set; }
        }

        // Private attached property (never seen in XAML or anywhere else) to attach
        // the state variable for us. Because this isn't used in XAML, we don't need
        // the normal GetXXX and SetXXX static methods.
        static readonly DependencyProperty RichEditChangeStateHelperProperty =
          DependencyProperty.RegisterAttached("RichEditChangeStateHelper",
          typeof(RichEditChangeStateHelper), typeof(CodeEditorControlExtension), null);

        // Inject our state into the textbox, and also attach an event-handler
        // for the TextChanged event.
        static void AttachCodeEditorControlChangingHelper(DependencyObject o)
        {
            if (o.GetValue(RichEditChangeStateHelperProperty) != null)
                return;

            var richEdit = o as CodeEditorControl;
            var helper = new RichEditChangeStateHelper();
            o.SetValue(RichEditChangeStateHelperProperty, helper);

            richEdit.Editor.Modified += (Editor sender, ModifiedEventArgs args) => {
                switch (args.ModificationType)
                {
                    case (int)MicaEditor.ModificationFlags.Container:
                    case (int)MicaEditor.ModificationFlags.EventMaskAll:
                    case (int)MicaEditor.ModificationFlags.ChangeTabStops:
                    case (int)MicaEditor.ModificationFlags.InsertCheck:
                    case (int)MicaEditor.ModificationFlags.ChangeEOLAnnotation:
                    case (int)MicaEditor.ModificationFlags.LexerState:
                    case (int)MicaEditor.ModificationFlags.ChangeStyle:
                    case (int)MicaEditor.ModificationFlags.ChangeFold:
                    case (int)MicaEditor.ModificationFlags.ChangeMarker:
                    case (int)MicaEditor.ModificationFlags.StartAction:
                    case (int)MicaEditor.ModificationFlags.ChangeIndicator:
                    case (int)MicaEditor.ModificationFlags.ChangeLineState:
                    case (int)MicaEditor.ModificationFlags.ChangeMargin:
                    case (int)MicaEditor.ModificationFlags.ChangeAnnotation:
                    case (int)MicaEditor.ModificationFlags.None:
                        return;
                    case (int)MicaEditor.ModificationFlags.InsertText:
                    case (int)MicaEditor.ModificationFlags.DeleteText:
                    case (int)MicaEditor.ModificationFlags.User:
                    case (int)MicaEditor.ModificationFlags.Undo:
                    case (int)MicaEditor.ModificationFlags.Redo:
                    case (int)MicaEditor.ModificationFlags.MultiStepUndoRedo:
                    case (int)MicaEditor.ModificationFlags.LastStepInUndoRedo:
                    case (int)MicaEditor.ModificationFlags.BeforeInsert:
                    case (int)MicaEditor.ModificationFlags.BeforeDelete:
                    case (int)MicaEditor.ModificationFlags.MultilineUndoRedo:
                        break;
                }

                string text = "";
                for (long i = 0; i < sender.LineCount; i++)
                {
                    var line = sender.GetLine(i);
                    text = text + line; // .AddToDelimiter(line, "\n");
                }
                text = text.Replace("\r", "\n");

                EdmsTasks.InsertJob(()=> { 
                    // To avoid re-entrancy, make sure we're not already changing
                    var state = GetState(o);
                    switch (state) {
                        case RichEditChangeState.Idle:
                            {
                                if (text != GetPlainText(o))
                                {
                                    SetState(o, RichEditChangeState.RichTextChanged);
                                    o.SetValue(PlainTextProperty, text);
                                }
                            }
                            break;
                        case RichEditChangeState.PlainTextChanged:
                            SetState(o, RichEditChangeState.Idle);
                            break;
                        default:
                            SetState(o, RichEditChangeState.Idle);
                            break;
                    }
                    
                }, true);

                ScriptNode_Editor.UpdateCollapseMargin(sender);
            };
        }

        // Helper to set the state managed by the textbox
        static void SetState(DependencyObject o, RichEditChangeState state)
        {
            (o.GetValue(RichEditChangeStateHelperProperty) as RichEditChangeStateHelper).State = state;
        }

        // Helper to get the state managed by the textbox
        static RichEditChangeState GetState(DependencyObject o)
        {
            return (o.GetValue(RichEditChangeStateHelperProperty) as RichEditChangeStateHelper).State;
        }
        #endregion
    }
#endif

    public class cweeSolidColorBrush
    {
        static cweeTask<SolidColorBrush> ct = cweeXamlHelper.ThemeColor("cweeDarkBlue");

        public cweeSolidColorBrush()
        {
            ct.ContinueWith(()=> {
                c = ct.Result;
            }, false);
        }

        public cweeSolidColorBrush(cweeTask<SolidColorBrush> d) { d.ContinueWith(()=> { c = d.Result; col = EdmsTasks.InsertJob(() => { return c.Color; }, true); }, false); }
        public cweeSolidColorBrush(SolidColorBrush d) { c = d; col = EdmsTasks.InsertJob(()=> { return c.Color; }, true); }

        public static implicit operator SolidColorBrush(cweeSolidColorBrush from)
        {
            return from.c;
        }
        public static implicit operator cweeSolidColorBrush(SolidColorBrush from)
        {
            return new cweeSolidColorBrush(from);
        }

        public static implicit operator cweeSolidColorBrush(cweeTask<SolidColorBrush> from)
        {
            return new cweeSolidColorBrush(from);
        }

        public SolidColorBrush c;
        public cweeTask<Color> col;

        public static cweeTask<Color> CreateBackground(List<SolidColorBrush> brushes)
        {
            List<Color> cols = new List<Color>(brushes.Count + 1);
            return EdmsTasks.InsertJob(() =>
            {
                foreach (var j in brushes)
                {
                    if (j != null) cols.Add(j.Color);
                }
            }, true, true).ContinueWith(()=> {
                Color toReturn = new Color() { R = 0, G = 0, B = 0, A = 0 };
                int numSamples = 0;
                float darkeningFactor = 0.05f;
                foreach (var r in cols)
                {
                    numSamples++;
                    if (numSamples == 1)
                    {
                        toReturn = r;
                    }
                    else
                    {
                        toReturn = toReturn.Lerp(r.Lerp(new Color() { A = 255, R = 0, G = 0, B = 0 }, darkeningFactor * 2.0f), 0.5f);
                    }
                }
                return toReturn;
            }, false);
        }
        public static cweeTask<Color> AvgColor(List<SolidColorBrush> brushes)
        {
            List<Color> cols = new List<Color>(brushes.Count + 1);
            return EdmsTasks.InsertJob(() => {
                foreach (var j in brushes) { if (j != null) cols.Add(j.Color); }
            }).ContinueWith(()=> {
                Color toReturn = new Color() { R = 0, G = 0, B = 0, A = 0 };
                double numSamples = 0;
                foreach (var r in cols)
                {
                    numSamples += toReturn.A / 255.0;

                    numSamples = Math.Max(numSamples, 1.0);

                    toReturn.R -= (byte)(toReturn.R / numSamples);// * opacity);
                    toReturn.R += (byte)(r.R / numSamples);// * opacity);

                    toReturn.G -= (byte)(toReturn.G / numSamples);// * opacity);
                    toReturn.G += (byte)(r.G / numSamples);// * opacity);

                    toReturn.B -= (byte)(toReturn.B / numSamples);// * opacity);
                    toReturn.B += (byte)(r.B / numSamples);// * opacity);

                    toReturn.A -= (byte)(toReturn.A / numSamples);// * opacity);
                    toReturn.A += (byte)(r.A / numSamples);// * opacity);
                }
                return toReturn;
            }, false);
        }
        public static cweeTask<Color> AvgColor(List<cweeTask<SolidColorBrush>> brushes)
        {
            List<Color> cols = new List<Color>(brushes.Count + 1);
            return EdmsTasks.cweeTask.TrueWhenCompleted(brushes).ContinueWith(() => {
                foreach (var j in brushes) { if (j != null) cols.Add(j.Result.Color); }
            }, true).ContinueWith(() => {
                Color toReturn = new Color() { R = 0, G = 0, B = 0, A = 0 };
                double numSamples = 0;
                foreach (var r in cols)
                {
                    numSamples += toReturn.A / 255.0;

                    numSamples = Math.Max(numSamples, 1.0);

                    toReturn.R -= (byte)(toReturn.R / numSamples);// * opacity);
                    toReturn.R += (byte)(r.R / numSamples);// * opacity);

                    toReturn.G -= (byte)(toReturn.G / numSamples);// * opacity);
                    toReturn.G += (byte)(r.G / numSamples);// * opacity);

                    toReturn.B -= (byte)(toReturn.B / numSamples);// * opacity);
                    toReturn.B += (byte)(r.B / numSamples);// * opacity);

                    toReturn.A -= (byte)(toReturn.A / numSamples);// * opacity);
                    toReturn.A += (byte)(r.A / numSamples);// * opacity);
                }
                return toReturn;
            }, false);
        }
        public static cweeTask<Color> AvgColor(List<cweeSolidColorBrush> brushes)
        {
            List<cweeTask<Color>> cols = new List<cweeTask<Color>>(brushes.Count+1);
            foreach (var r in brushes) cols.Add(r.col);
            return EdmsTasks.cweeTask.TrueWhenCompleted(cols).ContinueWith(()=> {
                Color toReturn = new Color() { R = 0, G = 0, B = 0, A = 0 };
                double numSamples = 0;
                foreach (var r in cols)
                {
                    numSamples += toReturn.A / 255.0;

                    numSamples = Math.Max(numSamples, 1.0);

                    toReturn.R -= (byte)(toReturn.R / numSamples);// * opacity);
                    toReturn.R += (byte)(r.Result.R / numSamples);// * opacity);

                    toReturn.G -= (byte)(toReturn.G / numSamples);// * opacity);
                    toReturn.G += (byte)(r.Result.G / numSamples);// * opacity);

                    toReturn.B -= (byte)(toReturn.B / numSamples);// * opacity);
                    toReturn.B += (byte)(r.Result.B / numSamples);// * opacity);

                    toReturn.A -= (byte)(toReturn.A / numSamples);// * opacity);
                    toReturn.A += (byte)(r.Result.A / numSamples);// * opacity);
                }
                return toReturn;
            }, false);
        }
        public static cweeTask<Color> StratifyColor(List<SolidColorBrush> brushes)
        {
            List<Color> cols = new List<Color>(brushes.Count + 1);
            return EdmsTasks.InsertJob(() => { foreach (var j in brushes) { if (j != null) cols.Add(j.Color); } }, true).ContinueWith(()=> {
                Color toReturn = new Color() { R = 0, G = 0, B = 0, A = 0 };
                int numSamples = 0;
                foreach (var j in cols)
                {
                    numSamples++;
                    if (numSamples == 1)
                        toReturn = j;
                    else
                        toReturn = toReturn.Lerp(j, 0.5f);
                }
                return toReturn;
            }, false);
        }
        public static cweeTask<Color> StratifyColor(List<cweeTask<SolidColorBrush>> brushes)
        {
            List<Color> cols = new List<Color>(brushes.Count + 1);
            return EdmsTasks.cweeTask.TrueWhenCompleted(brushes).ContinueWith(() => {
                foreach (var j in brushes) { if (j != null) cols.Add(j.Result.Color); }
            }, true).ContinueWith(() => {
                Color toReturn = new Color() { R = 0, G = 0, B = 0, A = 0 };
                int numSamples = 0;
                foreach (var j in cols)
                {
                    numSamples++;
                    if (numSamples == 1)
                        toReturn = j;
                    else
                        toReturn = toReturn.Lerp(j, 0.5f);
                }
                return toReturn;
            }, false);
        }
        public static cweeTask<Color> StratifyColor(List<cweeSolidColorBrush> brushes)
        {
            List<cweeTask<Color>> cols = new List<cweeTask<Color>>(brushes.Count + 1);
            foreach (var r in brushes) cols.Add(r.col);
            return EdmsTasks.cweeTask.TrueWhenCompleted(cols).ContinueWith(() => {
                Color toReturn = new Color() { R = 0, G = 0, B = 0, A = 0 };
                int numSamples = 0;
                foreach (var r in cols)
                {
                    numSamples++;
                    if (numSamples == 1)
                        toReturn = r.Result;
                    else
                        toReturn = toReturn.Lerp(r.Result, 0.5f);
                }
                return toReturn;
            }, false);
        }

        public static Color StratifyColor_IsMain(List<SolidColorBrush> brushes)
        {
            List<Color> cols = new List<Color>(brushes.Count + 1);
            foreach (var j in brushes) { if (j != null) cols.Add(j.Color); }

            Color toReturn = new Color() { R = 0, G = 0, B = 0, A = 0 };
            int numSamples = 0;
            foreach (var j in cols)
            {
                numSamples++;
                if (numSamples == 1)
                    toReturn = j;
                else
                    toReturn = toReturn.Lerp(j, 0.5f);
            }
            return toReturn;
        }
    }

    public class FormatCharDetails : IEquatable<FormatCharDetails>
    {
        public FontStyle FontStyle;
        public List<cweeSolidColorBrush> BackgroundColor = new List<cweeSolidColorBrush>();
        public List<cweeSolidColorBrush> ForegroundColor = new List<cweeSolidColorBrush>();
        public Color AvgForegroundColor;
        public Color AvgBackgroundColor;
        public UnderlineType Underline;
        public FormatEffect Bold;
        public FormatEffect Italic;

        public FormatCharDetails() { }
        public static bool operator ==(FormatCharDetails obj1, FormatCharDetails obj2)
        {
            if (ReferenceEquals(obj1, obj2))
                return true;
            if (ReferenceEquals(obj1, null))
                return false;
            if (ReferenceEquals(obj2, null))
                return false;
            return obj1.Equals(obj2);
        }
        public static bool operator !=(FormatCharDetails obj1, FormatCharDetails obj2) => !(obj1 == obj2);
        public override bool Equals(object obj) => Equals(obj as FormatCharDetails);
        public bool Equals(FormatCharDetails a)
        {
            return a.FontStyle == FontStyle
                && a.AvgForegroundColor == AvgForegroundColor
                && a.AvgBackgroundColor == AvgBackgroundColor
                && a.Underline == Underline
                && a.Bold == Bold
                && a.Italic == Italic
            ;
        }
        public override int GetHashCode()
        {
            return FontStyle.GetHashCode() & ForegroundColor.GetHashCode() & BackgroundColor.GetHashCode() & Underline.GetHashCode() & Bold.GetHashCode() & Italic.GetHashCode();
        }
    }
    public class FormatStrDetails
    {
        public FormatStrDetails() { }
        public FormatStrDetails(FormatCharDetails c)
        {
            details = c;
        }
        public FormatStrDetails(FormatCharDetails c, int s, int e)
        {
            details = c;
            start = s;
            end = e;
        }

        public int start = 0;
        public int end = 0;
        public FormatCharDetails details;
    }
    public class FormatDetails
    {
        public FormatDetails(string t)
        {
            text = t; formatting = new List<FormatCharDetails>(t.Length + 1);
            for (int i = 0; i < t.Length; i++)
            {
                formatting.Add(new FormatCharDetails()
                {
                    FontStyle = FontStyle.Normal,
                    BackgroundColor = new List<cweeSolidColorBrush>() { cweeXamlHelper.ThemeColor("cweePageBackground") },
                    ForegroundColor = new List<cweeSolidColorBrush>() { cweeXamlHelper.ThemeColor("cweeDarkBlue") },
                    Underline = UnderlineType.None,
                    Bold = FormatEffect.Off,
                    Italic = FormatEffect.Off
                });
            }
        }
        public int curlyMinLevel;
        public string text;
        public List<FormatCharDetails> formatting;

        public List<FormatStrDetails> GetStrFormatting()
        {
            List<FormatStrDetails> toreturn = new List<FormatStrDetails>();
            FormatStrDetails prev = null;
            int start = 0; int end = formatting.Count;
            for (int i = start; i < end; i++)
            {
                if (prev == null) { prev = new FormatStrDetails(formatting[i], i, i + 1); }
                else
                {
                    if (prev.details == formatting[i])
                    {
                        prev.end++;
                    }
                    else
                    {
                        toreturn.Add(prev);
                        prev = new FormatStrDetails(formatting[i], i, i + 1);
                    }
                }
            }
            if (prev != null)
            {
                toreturn.Add(prev);
            }

            return toreturn;
        }
    }
    public sealed partial class ScriptNode_Editor : UserControl
    {
        public static AtomicInt ObjCount = new AtomicInt();

        public cweeEvent<ScriptingNodeResult> RunClickedEvent = new cweeEvent<ScriptingNodeResult>();

        public ScriptNode_EditorVM vm = new ScriptNode_EditorVM();
        const int maxUndos = 30;
        internal LinkedList<string> Undos = new LinkedList<string>();
        internal LinkedList<string> Redos = new LinkedList<string>();
        private string previousScript = "";
        static AtomicInt trackPointerLock = new AtomicInt();
        static Point prevPosition = new Point(0, 0);
        internal cweeDequeue formatDeq = new cweeDequeue();
        internal DateTime queueTm = DateTime.Now;
        double ThisFontSize;
        public ScriptNode_Editor(ScriptingNodeViewModel parentVM, double DesiredFontSize = 12) // ScriptEngine en, ScriptingNodeViewModel parentVM
        {
            ThisFontSize = DesiredFontSize;
            ObjCount.Increment();

            vm.ParentVM = parentVM;

            this.InitializeComponent();


            var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");

            var cpb = cweeXamlHelper.ThemeColor("cweePageBackground");

            //Editor.FontSize = ThisFontSize;
            Editor.FontFamily = new FontFamily("Segoe UI");
            Editor.FontStretch = FontStretch.Normal;
            Editor.IsTextScaleFactorEnabled = true;

            Editor.PreviewKeyDown += Editor_PreviewKeyDown;
            Editor.CharacterReceived += Editor_CharacterReceived; ;

            this.PointerEntered += ScriptNode_Editor_PointerEntered;
            this.PointerExited += ScriptNode_Editor_PointerExited;


            Editor.Editor.Modified += (Editor sender, ModifiedEventArgs args) => {
                switch (args.ModificationType)
                {
                    case (int)MicaEditor.ModificationFlags.Container:
                    case (int)MicaEditor.ModificationFlags.EventMaskAll:
                    case (int)MicaEditor.ModificationFlags.ChangeTabStops:
                    case (int)MicaEditor.ModificationFlags.InsertCheck:
                    case (int)MicaEditor.ModificationFlags.ChangeEOLAnnotation:
                    case (int)MicaEditor.ModificationFlags.LexerState:
                    case (int)MicaEditor.ModificationFlags.ChangeStyle:
                    case (int)MicaEditor.ModificationFlags.ChangeFold:
                    case (int)MicaEditor.ModificationFlags.ChangeMarker:
                    case (int)MicaEditor.ModificationFlags.StartAction:
                    case (int)MicaEditor.ModificationFlags.ChangeIndicator:
                    case (int)MicaEditor.ModificationFlags.ChangeLineState:
                    case (int)MicaEditor.ModificationFlags.ChangeMargin:
                    case (int)MicaEditor.ModificationFlags.ChangeAnnotation:
                    case (int)MicaEditor.ModificationFlags.None:
                        return;
                    case (int)MicaEditor.ModificationFlags.InsertText:
                    case (int)MicaEditor.ModificationFlags.DeleteText:
                    case (int)MicaEditor.ModificationFlags.User:
                    case (int)MicaEditor.ModificationFlags.Undo:
                    case (int)MicaEditor.ModificationFlags.Redo:
                    case (int)MicaEditor.ModificationFlags.MultiStepUndoRedo:
                    case (int)MicaEditor.ModificationFlags.LastStepInUndoRedo:
                    case (int)MicaEditor.ModificationFlags.BeforeInsert:
                    case (int)MicaEditor.ModificationFlags.BeforeDelete:
                    case (int)MicaEditor.ModificationFlags.MultilineUndoRedo:
                        break;
                }
                DoTextChanging();
            };


        }

        private static void ScriptNode_Editor_PointerExited(object sender, PointerRoutedEventArgs e)
        {
            ScriptNode_EditorVM.pointerTracker?.Stop();
            ScriptNode_EditorVM.pointerTracker = null;

            (sender as ScriptNode_Editor).vm.hoverProcessor?.Stop();
            (sender as ScriptNode_Editor).vm.hoverProcessor = null;
        }

        private static void ScriptNode_Editor_PointerEntered(object sender, PointerRoutedEventArgs e)
        {
            (sender as ScriptNode_Editor).vm.hoverProcessor?.Stop();
            (sender as ScriptNode_Editor).vm.hoverProcessor = new cweeTimer(0.3, () => {
                (sender as ScriptNode_Editor).TrackPointer();
            }, false);

            if (ScriptNode_EditorVM.pointerTracker == null || !ScriptNode_EditorVM.pointerTracker.IsActive())
            {
                ScriptNode_EditorVM.pointerTracker = new cweeTimer(0.1, () => {
                    Point p = new Point(-1, -1);
                    Rect B = new Rect();
                    if (ScriptNode_EditorVM.pointerPositionLock.TryIncrementTo(1))
                    {
                        var job1 = EdmsTasks.InsertJob(() => {
                            try {
                                Point? pp = Window.Current?.CoreWindow?.PointerPosition;
                                if (pp.HasValue) {
                                    p.X = pp.Value.X;
                                    p.Y = pp.Value.Y;
                                    B = Window.Current.Bounds;
                                }
                            } catch (Exception) { }
                        }, true).ContinueWith(() => {
                            try {
                                if (p.X != -1 && p.Y != -1) {
                                    ScriptNode_EditorVM.pointerPosition.X = p.X - B.X;
                                    ScriptNode_EditorVM.pointerPosition.Y = p.Y - B.Y;
                                }
                            } catch (Exception) { }
                        }, false).ContinueWith(() => {
                            ScriptNode_EditorVM.pointerPositionLock.Decrement();
                        }, false);
                    }
                }, false);
            }
        }

        ~ScriptNode_Editor()
        {
            ObjCount.Decrement();

            vm = null;
            Undos = null;
            Redos = null;
            formatDeq?.Cancel();
            formatDeq = null;
        }

        private bool IsPressed(VirtualKey key) {
            return ((Window.Current.CoreWindow.GetKeyState(key) & Windows.UI.Core.CoreVirtualKeyStates.Down) == CoreVirtualKeyStates.Down);
        }




        public class cweeTipFlyoutViewModel : ViewModelBase
        {
            public int startingPosition = int.MinValue;
            public int currentPosition = int.MaxValue;

            private string _written = "";
            public string written
            {
                get
                {
                    return _written;
                }
                set
                {
                    _written = value;
                    OnPropertyChanged("written");
                }
            }
            public bool dotAccess = false;
            public string typeHint = "";
            public string nodeText = "";


            private ObservableCollection<UIElement> _tipElements;
            public ObservableCollection<UIElement> tipElements
            {
                get
                {
                    return _tipElements;
                }
                set
                {
                    _tipElements = value;
                    OnPropertyChanged("tipElements");
                }
            }
            public List<string> orig_functions;
        }
        public class cweeTipFlyout : Flyout
        {
            public cweeTipFlyoutViewModel vm = new cweeTipFlyoutViewModel();
        }








        public static void SetTipFlyout(CodeEditorControl obj, ScriptingNodeViewModel vm, int carotPosition, char characterAdded, string typeHint, Point? position = null)
        {
            cweeTipFlyout toFly = null;
            if (obj.ContextFlyout != null && obj.ContextFlyout is cweeTipFlyout)
            {
                toFly = obj.ContextFlyout as cweeTipFlyout;
            }
            if (toFly != null)
            {
                if (toFly.vm.startingPosition > carotPosition)
                {
                    toFly = null;
                }
            }
            string currentWritten = "";
            if (toFly == null)
            {
                toFly = new cweeTipFlyout();
                toFly.vm.startingPosition = carotPosition;
                toFly.Placement = Windows.UI.Xaml.Controls.Primitives.FlyoutPlacementMode.BottomEdgeAlignedLeft;
                toFly.ShowMode = Windows.UI.Xaml.Controls.Primitives.FlyoutShowMode.Standard;
                toFly.LightDismissOverlayMode = LightDismissOverlayMode.Off;
                toFly.AllowFocusWhenDisabled = false;
                toFly.AllowFocusOnInteraction = false;
                toFly.OverlayInputPassThroughElement = obj;
                toFly.vm.typeHint = typeHint;
                toFly.vm.currentPosition = carotPosition;
                currentWritten = (characterAdded.IsAlphaNumeric() ? $"{characterAdded}" : "");
                toFly.vm.written = currentWritten;
                toFly.vm.dotAccess = !characterAdded.IsAlphaNumeric();

                if (double.TryParse(toFly.vm.written, out double x))
                {
                    // we parsed this as a number -- it can't be an ID. 
                    CloseTipFlyout(obj);
                    return;
                }

                if (string.IsNullOrEmpty(typeHint)) { // NO TYPE KNOWN
                    if (!string.IsNullOrEmpty(currentWritten)) { // USER WROTE SOMETHING
                        toFly.vm.orig_functions = vm.ParentVM.engine.DoScript_Cast_VectorStrings($"\"{currentWritten}\".get_functions_that_start_with").OrderBy((string Y) => { string comp = Y.ToLower(); if (comp.CompareTo("A") < 0) { comp = "z" + comp; } return comp; }).ToList();
                    }
                    else {
                        // USER WROTE NOTHING
                        CloseTipFlyout(obj);
                        return;
                    }
                }
                else { // KNOWN/SUSPECTED TYPE
                    toFly.vm.orig_functions = vm.ParentVM.engine.DoScript_Cast_VectorStrings($"\"{typeHint}\".get_compatible_functions.keys").OrderBy((string Y) => { string comp = Y.ToLower(); if (comp.CompareTo("A") < 0) { comp = "z" + comp; } return comp; }).ToList();
                }

                obj.ContextFlyout = toFly;

                var temp = new ObservableCollection<UIElement>();
                foreach (var w in toFly.vm.orig_functions)
                {
                    string y = w;
                    var tb = cweeXamlHelper.SimpleTextBlock(y);
                    {
                        tb.Padding = new Thickness(0);
                        tb.Margin = new Thickness(0);
                    }
                    temp.Add(tb);
                }
                toFly.vm.tipElements = temp;

                Grid content = null;
                {
                    content = new Grid()
                    {
                        MinHeight = 20
                        , MinWidth = 20
                        , BorderBrush = new SolidColorBrush(Color.FromArgb(255, 0, 0, 0))
                        , BorderThickness = new Thickness(1)
                        , Margin = new Thickness(0)
                        , Padding = new Thickness(0)
                    };
                }
                ListView p = new ListView()
                {
                    HorizontalContentAlignment = HorizontalAlignment.Left,
                    VerticalContentAlignment = VerticalAlignment.Center,
                    ItemContainerStyle = cweeXamlHelper.StaticStyleResource("cweeListViewSimpleItemStyle"),
                    HorizontalAlignment = HorizontalAlignment.Left,
                    Padding = new Thickness(0, 0, 0, 0),
                    Margin = new Thickness(0),
                    MaxHeight = 500,
                    MaxWidth = 400
                };
                p.SetBinding(ListView.ItemsSourceProperty, new Binding() { 
                    Source = toFly.vm, 
                    Path = new PropertyPath("tipElements"), 
                    Mode = BindingMode.TwoWay, 
                    UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged 
                });
                content.Children.Add(p);

                toFly.Content = content;


                for (int i = toFly.vm.tipElements.Count - 1; i >=0; i--)
                {
                    UIElement contained = toFly.vm.tipElements[i];
                    var f = cweeXamlHelper.GetTextFromContainers(contained);
                    if (!string.IsNullOrEmpty(f) && (f.Contains(currentWritten, StringComparison.InvariantCultureIgnoreCase) || string.IsNullOrEmpty(currentWritten))) { // .StartsWith
                        continue;
                    }
                    else
                    {
                        toFly.vm.tipElements.RemoveAt(i);
                    }
                }

                //if (!string.IsNullOrEmpty(currentWritten)) {
                //    toFly.vm.tipElements = new ObservableCollection<UIElement>(toFly.vm.tipElements.OrderBy((contained) => {
                //        var f = cweeXamlHelper.GetTextFromContainers(contained);
                //        int distance = WaterWatch.LevenshteinDistance(f, currentWritten, false);
                //        distance -= (f.StartsWith(currentWritten, StringComparison.InvariantCultureIgnoreCase) ? 1 : 0);
                //        return distance;
                //    }));
                //}
            }
            else
            {
                toFly.vm.currentPosition = carotPosition;

                string plainText = CodeEditorControlExtension.GetPlainText(obj);
                try {
                    currentWritten = plainText.Mid(toFly.vm.startingPosition,
                        carotPosition - toFly.vm.startingPosition) + (characterAdded.IsAlphaNumeric() ? $"{characterAdded}" : "");
                }
                catch (Exception) {
                    currentWritten = (characterAdded.IsAlphaNumeric() ? $"{characterAdded}" : "");
                }
                toFly.vm.written = currentWritten;

                if (double.TryParse(toFly.vm.written, out double x))
                {
                    // we parsed this as a number -- it can't be an ID. 
                    CloseTipFlyout(obj);
                    return;
                }

                if (characterAdded == '\b')
                {
                    // re-make the tipelements

                    var temp = new ObservableCollection<UIElement>();
                    foreach (var w in toFly.vm.orig_functions)
                    {
                        string y = w;
                        var tb = cweeXamlHelper.SimpleTextBlock(y);
                        {
                            tb.Padding = new Thickness(0);
                            tb.Margin = new Thickness(0);
                        }
                        toFly.vm.tipElements.Add(tb);
                    }
                    toFly.vm.tipElements = temp;

                    Grid content = null;
                    {
                        content = new Grid()
                        {
                            MinHeight = 20
                            , MinWidth = 20
                            , BorderBrush = new SolidColorBrush(Color.FromArgb(255, 0, 0, 0))
                            , BorderThickness = new Thickness(1)
                            , Margin = new Thickness(0)
                            , Padding = new Thickness(0)
                        };
                    }
                    ListView p = new ListView()
                    {
                        HorizontalContentAlignment = HorizontalAlignment.Left,
                        VerticalContentAlignment = VerticalAlignment.Center,
                        ItemContainerStyle = cweeXamlHelper.StaticStyleResource("cweeListViewSimpleItemStyle"),
                        HorizontalAlignment = HorizontalAlignment.Left,
                        Padding = new Thickness(0, 0, 0, 0),
                        Margin = new Thickness(0),
                        MaxHeight = 500,
                        MaxWidth = 400
                    };
                    p.SetBinding(ListView.ItemsSourceProperty, new Binding() { 
                        Source = toFly.vm, 
                        Path = new PropertyPath("tipElements"), 
                        Mode = BindingMode.TwoWay, 
                        UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged 
                    });  // p.ItemsSource = toFly.vm.tipElements;
                    content.Children.Add(p);

                    toFly.Content = content;

                }

                for (int i = toFly.vm.tipElements.Count - 1; i >= 0; i--)
                {
                    UIElement contained = toFly.vm.tipElements[i];
                    var f = cweeXamlHelper.GetTextFromContainers(contained);
                    if (!string.IsNullOrEmpty(f) && (f.Contains(currentWritten, StringComparison.InvariantCultureIgnoreCase) || string.IsNullOrEmpty(currentWritten))) { // StartsWith
                        continue;
                    }
                    else {
                        toFly.vm.tipElements.RemoveAt(i);
                    }
                }

                //if (!string.IsNullOrEmpty(currentWritten))
                //{
                //    toFly.vm.tipElements = new ObservableCollection<UIElement>(toFly.vm.tipElements.OrderBy((contained) => {
                //        var f = cweeXamlHelper.GetTextFromContainers(contained);
                //        int distance = WaterWatch.LevenshteinDistance(f, currentWritten, false);
                //        distance -= (f.StartsWith(currentWritten, StringComparison.InvariantCultureIgnoreCase) ? 1 : 0);
                //        return distance;
                //    }));
                //}
            }

            if (toFly.vm.tipElements.Count <= 0)
            {
                CloseTipFlyout(obj);
                return;
            }

            if (!toFly.IsOpen)
            {
                toFly.ShowAt(obj, new Windows.UI.Xaml.Controls.Primitives.FlyoutShowOptions()
                {
                    ShowMode = Windows.UI.Xaml.Controls.Primitives.FlyoutShowMode.TransientWithDismissOnPointerMoveAway,
                    Placement = Windows.UI.Xaml.Controls.Primitives.FlyoutPlacementMode.BottomEdgeAlignedLeft,
                    Position = position
                });
            }
        }
        public static bool TipFlyoutVisible(CodeEditorControl obj)
        {
            cweeTipFlyout toFly = null;
            if (obj.ContextFlyout != null && obj.ContextFlyout is cweeTipFlyout)
            {
                toFly = obj.ContextFlyout as cweeTipFlyout;
            }
            if (toFly != null)
            {
                return toFly.IsOpen;
            }
            return false;
        }
        public static void CloseTipFlyout(CodeEditorControl obj) {
            cweeTipFlyout toFly = null;
            if (obj.ContextFlyout != null && obj.ContextFlyout is cweeTipFlyout)
            {
                toFly = obj.ContextFlyout as cweeTipFlyout;
            }
            if (toFly != null) {
                toFly.Hide();
                obj.ContextFlyout = null;
            }
        }
        public static bool AcceptTipFlyout(CodeEditorControl obj, ScriptingNodeViewModel vm)
        {
            cweeTipFlyout toFly = null;
            if (obj.ContextFlyout != null && obj.ContextFlyout is cweeTipFlyout)
            {
                toFly = obj.ContextFlyout as cweeTipFlyout;
            }
            if (toFly != null)
            {
                if (toFly.vm.tipElements.Count > 0) {
                    string recommended = cweeXamlHelper.GetTextFromContainers(toFly.vm.tipElements[0]);
                    // do we know if the 'recommendation' is a type or a function? 
                    bool is_type = recommended == vm.ParentVM.engine.DoScript($"\"{recommended}\".type.to_string");
                    
                    if (!is_type)
                    {
                        string innerContent = "";

                        vector_string param_names;
                        vector_string param_types = null;
                        if (toFly.vm.dotAccess && !string.IsNullOrEmpty(toFly.vm.typeHint)) {
                            param_names = vm.ParentVM.engine.DoScript_Cast_VectorStrings($"{recommended}.get_function_param_names(\"{toFly.vm.typeHint}\").to_string.Replace(\"[\",\"\").Replace(\"]\",\"\").Split(\", \")"); // the names of all the input params.

                            int num = param_names.Count;
                            for (int i = 0; i < num; i++)
                            {
                                try
                                {
                                    if (param_names[i] == $"param{i + 1}") {
                                        if (param_types == null) param_types = vm.ParentVM.engine.DoScript_Cast_VectorStrings($"{recommended}.get_param_types.to_string.Replace(\"[\",\"\").Replace(\"]\",\"\").Split(\", \")"); // includes the return type followed by the input param types.
                                        int count_params = param_types.Count;
                                        if (count_params > (i + 2))
                                        {
                                            innerContent = innerContent.AddToDelimiter(param_types[i + 2], ", ");
                                        }
                                        else
                                        {
                                            // we need the "contained" functions and to try from there
                                            innerContent = innerContent.AddToDelimiter($"param{i + 1}", ", ");
                                        }
                                    }
                                    else {
                                        innerContent = innerContent.AddToDelimiter(param_names[i], ", ");
                                    }
                                }
                                catch (Exception)
                                {
                                    innerContent = innerContent.AddToDelimiter($"param{i + 1}", ", ");
                                }
                            }
                            
                        }
                        else {
                            param_names = vm.ParentVM.engine.DoScript_Cast_VectorStrings($"{recommended}.get_function_param_names"); // the names of all the input params.

                            int num = param_names.Count;
                            if (toFly.vm.dotAccess)
                            {
                                for (int i = 1; i < num; i++)
                                {
                                    try
                                    {
                                        if (param_names[i] == $"param{i}")
                                        {
                                            if (param_types == null) param_types = vm.ParentVM.engine.DoScript_Cast_VectorStrings($"{recommended}.get_param_types.to_string.Replace(\"[\",\"\").Replace(\"]\",\"\").Split(\", \")"); // includes the return type followed by the input param types.
                                            int count_params = param_types.Count;
                                            if (count_params > (i + 1))
                                            {
                                                innerContent = innerContent.AddToDelimiter(param_types[i + 1], ", ");
                                            }
                                            else
                                            {
                                                // we need the "contained" functions and to try from there


                                                innerContent = innerContent.AddToDelimiter($"param{i}", ", ");
                                            }
                                        }
                                        else
                                        {
                                            innerContent = innerContent.AddToDelimiter(param_names[i], ", ");
                                        }
                                    }
                                    catch (Exception)
                                    {
                                        innerContent = innerContent.AddToDelimiter($"param{i}", ", ");
                                    }
                                }
                            }
                            else
                            {
                                for (int i = 0; i < num; i++)
                                {
                                    try
                                    {
                                        if (param_names[i] == $"param{i}")
                                        {
                                            if (param_types == null) param_types = vm.ParentVM.engine.DoScript_Cast_VectorStrings($"{recommended}.get_param_types.to_string.Replace(\"[\",\"\").Replace(\"]\",\"\").Split(\", \")"); // includes the return type followed by the input param types.
                                            int count_params = param_types.Count;
                                            if (count_params > (i + 1))
                                            {
                                                innerContent = innerContent.AddToDelimiter(param_types[i + 1], ", ");
                                            }
                                            else
                                            {
                                                innerContent = innerContent.AddToDelimiter($"param{i}", ", ");
                                            }
                                        }
                                        else
                                        {
                                            innerContent = innerContent.AddToDelimiter(param_names[i], ", ");
                                        }
                                    }
                                    catch (Exception)
                                    {
                                        innerContent = innerContent.AddToDelimiter($"param{i}", ", ");
                                    }
                                }
                            }
                        }
                        
                        if (!string.IsNullOrEmpty(innerContent))
                        {
                            recommended += $"({innerContent})";
                        }
                        else
                        {
                            if (!toFly.vm.dotAccess)
                            {
                                recommended += "()";
                            }
                        }     
                    }

                    

                    if (toFly.vm.currentPosition != toFly.vm.startingPosition || toFly.vm.written.Length > 0)
                    {
                        obj.Editor.DeleteRange(toFly.vm.startingPosition, 
                            // 1 + (toFly.vm.currentPosition - toFly.vm.startingPosition)
                            (string.IsNullOrEmpty(toFly.vm.written) ? (1 + (toFly.vm.currentPosition - toFly.vm.startingPosition)) : toFly.vm.written.Length)
                        );
                    }
                    obj.Editor.InsertText(toFly.vm.startingPosition, recommended);
                    obj.Editor.GotoPos(toFly.vm.startingPosition + recommended.Length);

                    return true;
                }
            }

            return false;
        }



        
        public static AtomicInt Is_Processing_KeyDown = new AtomicInt();
        private void Editor_PreviewKeyDown(object sender, KeyRoutedEventArgs e) {
            Is_Processing_KeyDown.Increment();
            try
            {             
                int keyCode = (int)e.Key;
                CodeEditorControl tb = sender as CodeEditorControl;

                

                if (e.Key == VirtualKey.Enter || e.Key == VirtualKey.Tab)
                {
                    // accept the input
                    e.Handled = AcceptTipFlyout(tb, vm.ParentVM);
                    CloseTipFlyout(tb);
                }
                if (e.Key == VirtualKey.Back)
                {
                    // SPECIAL CASE, IF THE FLYOUT IS VISIBLE
                    if (TipFlyoutVisible(tb) && tb.Editor.CurrentPos > 0)
                    {
                        int currentPosition = (int)tb.Editor.CurrentPos;
                        SetTipFlyout(tb, vm.ParentVM, currentPosition-1, '\b', null, new Point(tb.Editor.PointXFromPosition(currentPosition-1), 24 + tb.Editor.PointYFromPosition(currentPosition-1)));
                    }
                }
                if (e.Key == VirtualKey.Up)
                {
                    // TODO
                }
                if (e.Key == VirtualKey.Down)
                {
                    // TODO
                }
                if (e.Key == VirtualKey.Space || e.Key == VirtualKey.Tab || e.Key == VirtualKey.Left || e.Key == VirtualKey.Right || e.Key == VirtualKey.Escape || e.Key == VirtualKey.GoBack || e.Key == VirtualKey.Delete || e.Key == VirtualKey.Home || e.Key == VirtualKey.End)
                {
                    CloseTipFlyout(tb);
                }
            }
            catch (Exception) { }
            finally
            {
                Is_Processing_KeyDown.Decrement();
            }

        }


        private void Editor_CharacterReceived(UIElement sender, CharacterReceivedRoutedEventArgs args)
        {
            Is_Processing_KeyDown.Increment();
            try
            {
                char keyCode = args.Character;
                try
                {
                    CodeEditorControl tb = sender as CodeEditorControl;
                    int currentPosition = (int)tb.Editor.CurrentPos;

                    if (!keyCode.IsAlphaNumeric() && keyCode != '.')
                    {
                        CloseTipFlyout(tb);
                        return;
                    }

                    if (currentPosition >= 0)
                    {
                        var nodes = FindNodesAtPosition(Math.Max(0, currentPosition /*- 1*/), true);
                        char prevChar = (char)tb.Editor.GetCharAt(currentPosition);
                        if (nodes != null && nodes.Count > 0)
                        {
                            var node = nodes[0];
                            if (node != null)
                            {
                                if (node.type_get() != WaterWatchEnums.ScriptNodeType.Constant)
                                {
                                    string typeHint = node.typeHint_get();
                                    if (prevChar == '(' || prevChar == '{') {
                                        typeHint = "";
                                    }

                                    if (string.IsNullOrEmpty(typeHint))
                                    {
                                        if (keyCode == '.')
                                        {
                                            // user entered '.' -- does this previous node at least have text?
                                            string nodeText = node.text_get();
                                            if (string.IsNullOrEmpty(nodeText)) {
                                                // no clue... nothing to do.
                                                CloseTipFlyout(tb);
                                            }
                                            else {
                                                CloseTipFlyout(tb);
                                                SetTipFlyout(tb, vm.ParentVM, currentPosition + 1, keyCode, nodeText, new Point(tb.Editor.PointXFromPosition(currentPosition), 24 + tb.Editor.PointYFromPosition(currentPosition)));
                                            }
                                        }
                                        else
                                        {
                                            // no clue what this is... free function? 
                                            SetTipFlyout(tb, vm.ParentVM, currentPosition, keyCode, null, new Point(tb.Editor.PointXFromPosition(currentPosition), 24 + tb.Editor.PointYFromPosition(currentPosition)));
                                        }
                                    }
                                    else
                                    {
                                        if (keyCode == '.')
                                        { // accessing the functions from the previous ID (probably... could be a number!) // in almost all contexts, this will be a new flyout, and not an extension of a previous one.
                                            CloseTipFlyout(tb);
                                            

                                            SetTipFlyout(tb, vm.ParentVM, currentPosition + 1, keyCode, typeHint, new Point(tb.Editor.PointXFromPosition(currentPosition), 24 + tb.Editor.PointYFromPosition(currentPosition)));
                                        }
                                        if (keyCode.IsAlphaNumeric())
                                        { // writing an ID of some type (probably...)
                                            SetTipFlyout(tb, vm.ParentVM, currentPosition, keyCode, typeHint, new Point(tb.Editor.PointXFromPosition(currentPosition), 24 + tb.Editor.PointYFromPosition(currentPosition)));
                                        }
                                    }
                                }
                                else
                                {
                                    CloseTipFlyout(tb);
                                }
                            }
                            else
                            {
                                // no clue what this is... free function? 
                                SetTipFlyout(tb, vm.ParentVM, currentPosition, keyCode, null, new Point(tb.Editor.PointXFromPosition(currentPosition), 24 + tb.Editor.PointYFromPosition(currentPosition)));
                            }
                        }
                        else
                        {
                            if (keyCode.IsAlphaNumeric())
                            { // no clue what this is... free function? 
                                SetTipFlyout(tb, vm.ParentVM, currentPosition, keyCode, null, new Point(tb.Editor.PointXFromPosition(currentPosition), 24 + tb.Editor.PointYFromPosition(currentPosition)));
                                return;
                            }
                        }
                    }
                    else
                    {
                        CloseTipFlyout(tb);
                    }
                }
                catch (Exception) { }
                finally { }
            }
            catch (Exception) { }
            finally
            {
                Is_Processing_KeyDown.Decrement();
            }

        }



        int defaultZoom;
        static public string DelineatedString(List<string> list)
        {
            string toReturn = "";
            foreach (var x in list)
            {
                toReturn = toReturn.AddToDelimiter(x, " ");
            }
            return toReturn;
        }

        private void Editor_OnLoaded(object sender, RoutedEventArgs e)
        {
            CodeEditorControl tb = sender as CodeEditorControl;
            tb.Loaded -= Editor_OnLoaded;

            tb.Editor.ZoomChanged += Editor_ZoomChanged;

            defaultZoom = tb.Editor.Zoom;

            tb.HighlightingLanguage = "cpp";

            tb.Editor.SetDefaultFoldDisplayText("...");
            tb.Editor.FoldDisplayTextStyle = MicaEditor.FoldDisplayTextStyle.Boxed;
            tb.Editor.AutomaticFold = MicaEditor.AutomaticFold.Click;
            // tb.Editor.SetFoldFlags(MicaEditor.FoldFlag.LineAfterContracted); // GOOD
            tb.Editor.SetMarginSensitiveN(0, true); // enable user to click on row-number margin

            tb.Editor.SetMarginWidthN(2, 7);
            tb.Editor.SetMarginTypeN(2, MicaEditor.MarginType.Text);
            tb.Editor.SetMarginSensitiveN(2, true);
            tb.Editor.SetMarginMaskN(2, (int)MicaEditor.MarkerSymbol.Arrow);

            tb.Editor.IndicSetStyle(2, MicaEditor.IndicatorStyle.SquigglePixmap); // works good
            tb.Editor.IndicSetFore(2, 0x0000ff); // (light red)

            tb.Editor.ConvertEOLs(EndOfLine.Lf);
            tb.Editor.EOLMode = EndOfLine.Lf;

            tb.Editor.FontQuality = FontQuality.QualityAntialiased;

            tb.Editor.IndicSetHoverFore(2, 0xf000ff); // needed to unlock the "click" ability
            tb.Editor.IndicatorClick += Editor_IndicatorClick;

            tb.Editor.MouseDwellTime = 500;
            tb.Editor.DwellStart += Editor_DwellStart;
            tb.Editor.DwellEnd += Editor_DwellEnd;
            tb.PointerEntered += Tb_PointerEntered;
            tb.PointerExited += Tb_PointerExited;

            tb.Editor.MarginClick += (MicaEditor.Editor sender2, MicaEditor.MarginClickEventArgs args2) => {
                if (args2.Margin == 0 || args2.Margin == 2)
                {
                    long line = sender2.LineFromPosition(args2.Position);
                    sender2.FoldLine(line, MicaEditor.FoldAction.Toggle);
                    UpdateCollapseMargin(sender2);
                }
            };

            List<string> BuiltInTypes = new List<string>() {
                "auto", "var", "int", "float",
                "true", "false", "double", "long",
                "bool", "null", "global", "_", "__LINE__", 
                "__FILE__", "__FUNC__", "__CLASS__", "__LOCK__"
            };
            List<string> LanguageFunctions = new List<string>() { 
                "def", "fun", "while", "for", "async", "Async",
                "if", "else", "return", "break", "class", 
                "attr", "switch", "case", "continue", "default",
                "try", "catch", "do", "finally"
            };

            tb.Editor.SetKeyWords(0, DelineatedString(BuiltInTypes)); // built-in types
            tb.Editor.SetKeyWords(1, DelineatedString(LanguageFunctions)); // language functions


            // vm?.ParentVM?.ParentVM?.engine?.DoScript_Cast_VectorStrings("TypeNames");
            var function_names = vm?.ParentVM?.ParentVM?.engine?.DoScript_Cast_VectorStrings("FunctionNames()");
            if (function_names != null) {
                string delinString = DelineatedString(function_names);
                foreach (char x in "()[]{}=,:;!%^&*|?~+-") // allowed around function names, so they can't be a part of function names!
                {
                    delinString = delinString.Replace(x, ' ');
                }          
                tb.Editor.SetKeyWords(6, delinString); // user-defined functions
            }

            var type_names = vm?.ParentVM?.ParentVM?.engine?.DoScript_Cast_VectorStrings("TypeNames()");
            if (type_names != null)
            {
                string delinString = DelineatedString(type_names);
                foreach (char x in "()[]{}=,:;!%^&*|?~+-") // allowed around function names, so they can't be a part of function names!
                {
                    delinString = delinString.Replace(x, ' ');
                }
                tb.Editor.SetKeyWords(3, delinString);  // user-defined classes
            }

            DoTextChanging(true, true, "Created a new node.");            
        }

        private List<ScriptingNode> FindNodesAtPosition(int charNum, bool NoError = false)
        {
            List<ScriptingNode> foundNodes = new List<ScriptingNode>();
            var prevNodes = NoError ? vm.MostRecentParsedNodes_NoError : vm.MostRecentParsedNodes;
            if (prevNodes != null && charNum >= 0)
            {

                for (int i = prevNodes.Count - 1; i >= 0; i--)
                {
                    if (prevNodes[i].startColumn_get() <= charNum && prevNodes[i].endColumn_get() >= charNum)
                    {
                        if (
                        prevNodes[i].type_get() == WaterWatchEnums.ScriptNodeType.File ||
                        prevNodes[i].type_get() == WaterWatchEnums.ScriptNodeType.Noop)
                            continue;
                        if (
                        prevNodes[i].type_get() == WaterWatchEnums.ScriptNodeType.Id) { foundNodes = new List<ScriptingNode>(); }
                        foundNodes.Add(prevNodes[i]);
                    }
                }

                if (foundNodes.Count > 0)
                {
                    return foundNodes;
                }
                else return null;
            }
            else
            {
                return null;
            }
        }

        private int FindFirstDeviationBetweenCurrentScriptAndValidScript() {
            string currentText = CodeEditorControlExtension.GetPlainText(this.Editor);
            string noErrorText = vm.MostRecentParsedScript_NoError;

            int err = currentText.CompareTo(noErrorText);

            return err;
        }

        private Grid ReviewNodeList(List<ScriptingNode> nodes) {
            if (nodes != null && nodes.Count > 0)
            {
                Grid toReturn = new Grid(); {
                    toReturn.RowDefinitions.Add(new RowDefinition() { Height = new GridLength(1, GridUnitType.Auto) });
                    toReturn.RowDefinitions.Add(new RowDefinition() { Height = new GridLength(1, GridUnitType.Auto) });
                    toReturn.RowDefinitions.Add(new RowDefinition() { Height = new GridLength(1, GridUnitType.Auto) });
                    toReturn.RowDefinitions.Add(new RowDefinition() { Height = new GridLength(1, GridUnitType.Auto) });
                    toReturn.RowDefinitions.Add(new RowDefinition() { Height = new GridLength(1, GridUnitType.Auto) });
                    toReturn.RowDefinitions.Add(new RowDefinition() { Height = new GridLength(1, GridUnitType.Auto) });
                }
                {
                    BreadcrumbBar bcb = new BreadcrumbBar(); {
                        List<string> blocks = new List<string>();{
                            for (int i = nodes.Count - 1; i >= 0; i--) {
                                blocks.Add(nodes[i].type_get().ToString());
                            }
                        }
                        bcb.ItemsSource = blocks;
                    }
                    toReturn.Children.Add(bcb);
                    Grid.SetRow(bcb, 0);
                }
                {
                    switch (nodes[0].type_get())
                    {

                        case WaterWatchEnums.ScriptNodeType.Constant:
                            // number, string, etc. 
                            {
                                var fgc = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                                fgc.ContinueWith(() => {
                                    StackPanel p = new StackPanel() { Spacing = 5, Orientation = Orientation.Horizontal };
                                    {
                                        {
                                            SymbolIcon i;
                                            switch (nodes[0].typeHint_get())
                                            {
                                                case "string":
                                                    i = new SymbolIcon() { Symbol = Symbol.Font, Foreground = fgc.Result };
                                                    break;
                                                case "long_double":
                                                case "long":
                                                case "int64_t":
                                                case "double":
                                                case "float":
                                                case "int":
                                                    i = new SymbolIcon() { Symbol = Symbol.Calculator, Foreground = fgc.Result };
                                                    break;
                                                default:
                                                    i = new SymbolIcon() { Symbol = Symbol.ViewAll, Foreground = fgc.Result };
                                                    break;
                                            }
                                            p.Children.Add(i);
                                        }
                                        if (!string.IsNullOrEmpty(nodes[0].typeHint_get()))
                                        {
                                            TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].typeHint_get(), HorizontalAlignment.Left);
                                            tb.FontStyle = FontStyle.Italic;
                                            p.Children.Add(tb);
                                        }
                                        {
                                            TextBlock tb;
                                            switch (nodes[0].typeHint_get())
                                            {
                                                case "string":
                                                    tb = cweeXamlHelper.SimpleTextBlock("\"" + nodes[0].text_get() + "\"", HorizontalAlignment.Left);
                                                    break;
                                                default:
                                                    tb = cweeXamlHelper.SimpleTextBlock(nodes[0].text_get(), HorizontalAlignment.Left);
                                                    break;
                                            }
                                            p.Children.Add(tb);
                                        }
                                    }
                                    toReturn.Children.Add(p);
                                    Grid.SetRow(p, 1);
                                }, true);
                            }
                            break;

                        default:
                        case WaterWatchEnums.ScriptNodeType.If:
                        case WaterWatchEnums.ScriptNodeType.For:
                        case WaterWatchEnums.ScriptNodeType.Assign_Retroactively:
                        case WaterWatchEnums.ScriptNodeType.Unused_Return_Fun_Call:
                        case WaterWatchEnums.ScriptNodeType.Arg_List:                            
                        case WaterWatchEnums.ScriptNodeType.Array_Call:
                        case WaterWatchEnums.ScriptNodeType.Dot_Access:                            
                        case WaterWatchEnums.ScriptNodeType.Lambda:
                        case WaterWatchEnums.ScriptNodeType.Block:
                        case WaterWatchEnums.ScriptNodeType.Scopeless_Block:
                        case WaterWatchEnums.ScriptNodeType.Def:
                        case WaterWatchEnums.ScriptNodeType.While:
                        case WaterWatchEnums.ScriptNodeType.Ranged_For:
                        case WaterWatchEnums.ScriptNodeType.Inline_Array:
                        case WaterWatchEnums.ScriptNodeType.Inline_Map:
                        case WaterWatchEnums.ScriptNodeType.Return:
                        case WaterWatchEnums.ScriptNodeType.File:
                        case WaterWatchEnums.ScriptNodeType.Prefix:
                        case WaterWatchEnums.ScriptNodeType.Break:
                        case WaterWatchEnums.ScriptNodeType.Continue:
                        case WaterWatchEnums.ScriptNodeType.Map_Pair:
                        case WaterWatchEnums.ScriptNodeType.Value_Range:
                        case WaterWatchEnums.ScriptNodeType.Inline_Range:
                        case WaterWatchEnums.ScriptNodeType.Do:
                        case WaterWatchEnums.ScriptNodeType.Try:
                        case WaterWatchEnums.ScriptNodeType.Catch:
                        case WaterWatchEnums.ScriptNodeType.Finally:
                        case WaterWatchEnums.ScriptNodeType.Method:
                        case WaterWatchEnums.ScriptNodeType.Attr_Decl:
                        case WaterWatchEnums.ScriptNodeType.Logical_And:
                        case WaterWatchEnums.ScriptNodeType.Logical_Or:
                        case WaterWatchEnums.ScriptNodeType.Reference:
                        case WaterWatchEnums.ScriptNodeType.Switch:
                        case WaterWatchEnums.ScriptNodeType.Case:
                        case WaterWatchEnums.ScriptNodeType.Default:
                        case WaterWatchEnums.ScriptNodeType.Noop:
                        case WaterWatchEnums.ScriptNodeType.Class:
                        case WaterWatchEnums.ScriptNodeType.Binary:
                        case WaterWatchEnums.ScriptNodeType.Arg:
                        case WaterWatchEnums.ScriptNodeType.Global_Decl:                                                
                        case WaterWatchEnums.ScriptNodeType.ControlBlock:
                        case WaterWatchEnums.ScriptNodeType.Postfix:
                        case WaterWatchEnums.ScriptNodeType.Fun_Call:
                        case WaterWatchEnums.ScriptNodeType.Var_Decl:
                        case WaterWatchEnums.ScriptNodeType.Assign_Decl:
                            if (!string.IsNullOrEmpty(nodes[0].typeHint_get()) || !string.IsNullOrEmpty(nodes[0].text_get()))
                            {
                                var fgc = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                                fgc.ContinueWith(() => {
                                    StackPanel p = new StackPanel() { Spacing = 5, Orientation = Orientation.Horizontal };
                                    {
                                        if (!string.IsNullOrEmpty(nodes[0].typeHint_get()))
                                        {
                                            TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].typeHint_get(), HorizontalAlignment.Left);
                                            tb.FontStyle = FontStyle.Italic;
                                            p.Children.Add(tb);
                                        }
                                        if (!string.IsNullOrEmpty(nodes[0].text_get()))
                                        {
                                            TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].text_get(), HorizontalAlignment.Left);
                                            p.Children.Add(tb);
                                        }
                                    }
                                    toReturn.Children.Add(p);
                                    Grid.SetRow(p, 1);
                                }, true);
                            }
                            break;

                        case WaterWatchEnums.ScriptNodeType.Id:
                            // class names, function names, variable names, etc.
                            {
                                var fgc = cweeXamlHelper.ThemeColor("cweeDarkBlue");
                                if (nodes.Count > 1)
                                {
                                    int w = 1;
                                    while (nodes[w].type_get() == WaterWatchEnums.ScriptNodeType.Arg_List) w++;

                                    switch (nodes[w].type_get()) {
                                        default:
                                        case WaterWatchEnums.ScriptNodeType.Assign_Retroactively:
                                        case WaterWatchEnums.ScriptNodeType.Unused_Return_Fun_Call:
                                        case WaterWatchEnums.ScriptNodeType.Arg_List:
                                        case WaterWatchEnums.ScriptNodeType.Array_Call:                                        
                                        case WaterWatchEnums.ScriptNodeType.Lambda:
                                        case WaterWatchEnums.ScriptNodeType.Block:
                                        case WaterWatchEnums.ScriptNodeType.Scopeless_Block:
                                        case WaterWatchEnums.ScriptNodeType.Def:
                                        case WaterWatchEnums.ScriptNodeType.While:
                                        case WaterWatchEnums.ScriptNodeType.Inline_Array:
                                        case WaterWatchEnums.ScriptNodeType.Inline_Map:
                                        case WaterWatchEnums.ScriptNodeType.Return:
                                        case WaterWatchEnums.ScriptNodeType.File:
                                        case WaterWatchEnums.ScriptNodeType.Prefix:
                                        case WaterWatchEnums.ScriptNodeType.Break:
                                        case WaterWatchEnums.ScriptNodeType.Continue:
                                        case WaterWatchEnums.ScriptNodeType.Map_Pair:
                                        case WaterWatchEnums.ScriptNodeType.Value_Range:
                                        case WaterWatchEnums.ScriptNodeType.Inline_Range:
                                        case WaterWatchEnums.ScriptNodeType.Do:
                                        case WaterWatchEnums.ScriptNodeType.Try:
                                        case WaterWatchEnums.ScriptNodeType.Catch:
                                        case WaterWatchEnums.ScriptNodeType.Finally:
                                        case WaterWatchEnums.ScriptNodeType.Method:
                                        case WaterWatchEnums.ScriptNodeType.Attr_Decl:
                                        case WaterWatchEnums.ScriptNodeType.Logical_And:
                                        case WaterWatchEnums.ScriptNodeType.Logical_Or:                                        
                                        case WaterWatchEnums.ScriptNodeType.Switch:
                                        case WaterWatchEnums.ScriptNodeType.Case:
                                        case WaterWatchEnums.ScriptNodeType.Default:
                                        case WaterWatchEnums.ScriptNodeType.Noop:
                                        case WaterWatchEnums.ScriptNodeType.Class:
                                        case WaterWatchEnums.ScriptNodeType.Binary:
                                        case WaterWatchEnums.ScriptNodeType.Arg:                                        
                                        case WaterWatchEnums.ScriptNodeType.ControlBlock:
                                        case WaterWatchEnums.ScriptNodeType.Postfix:                                            
                                            fgc.ContinueWith(() => {
                                                StackPanel p = new StackPanel() { Spacing = 5, Orientation = Orientation.Horizontal };
                                                {
                                                    {
                                                        SymbolIcon i = new SymbolIcon() { Symbol = Symbol.Preview, Foreground = fgc.Result };
                                                        p.Children.Add(i);
                                                    }
                                                    if (!string.IsNullOrEmpty(nodes[0].typeHint_get())) {
                                                        TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].typeHint_get(), HorizontalAlignment.Left);
                                                        tb.FontStyle = FontStyle.Italic;
                                                        p.Children.Add(tb);
                                                    }
                                                    {
                                                        TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].text_get(), HorizontalAlignment.Left);
                                                        p.Children.Add(tb);
                                                    }
                                                }
                                                toReturn.Children.Add(p);
                                                Grid.SetRow(p, 1);
                                            }, true);
                                            break;

                                        case WaterWatchEnums.ScriptNodeType.Ranged_For:
                                        case WaterWatchEnums.ScriptNodeType.Global_Decl:
                                        case WaterWatchEnums.ScriptNodeType.Assign_Decl:
                                        case WaterWatchEnums.ScriptNodeType.Var_Decl:
                                        case WaterWatchEnums.ScriptNodeType.Reference:
                                            // variable / variable name
                                            fgc.ContinueWith(() => {
                                                StackPanel p = new StackPanel() { Spacing = 5, Orientation = Orientation.Horizontal };
                                                {
                                                    {
                                                        TextBlock tb = cweeXamlHelper.SimpleTextBlock("(Variable)", HorizontalAlignment.Left);
                                                        p.Children.Add(tb);
                                                    }
                                                    {
                                                        SymbolIcon i = new SymbolIcon() { Symbol = Symbol.Globe, Foreground = fgc.Result };
                                                        p.Children.Add(i);
                                                    }
                                                    if (!string.IsNullOrEmpty(nodes[0].typeHint_get()))
                                                    {
                                                        TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].typeHint_get(), HorizontalAlignment.Left);
                                                        tb.FontStyle = FontStyle.Italic;
                                                        p.Children.Add(tb);
                                                    }
                                                    {
                                                        TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].text_get(), HorizontalAlignment.Left);
                                                        p.Children.Add(tb);
                                                    }
                                                }
                                                toReturn.Children.Add(p);
                                                Grid.SetRow(p, 1);
                                            }, true);
                                            break;

                                        case WaterWatchEnums.ScriptNodeType.Fun_Call:
                                        case WaterWatchEnums.ScriptNodeType.Dot_Access:
                                            if (!string.IsNullOrEmpty(nodes[0].typeHint_get()) && nodes[0].typeHint_get() == nodes[0].text_get()) {
                                                // type name
                                                fgc.ContinueWith(() => {
                                                    {
                                                        StackPanel p = new StackPanel() { Spacing = 5, Orientation = Orientation.Horizontal };
                                                        {
                                                            {
                                                                TextBlock tb = cweeXamlHelper.SimpleTextBlock("(Typename)", HorizontalAlignment.Left);
                                                                p.Children.Add(tb);
                                                            }
                                                            {
                                                                SymbolIcon i = new SymbolIcon() { Symbol = Symbol.XboxOneConsole, Foreground = fgc.Result };
                                                                p.Children.Add(i);
                                                            }
                                                            {
                                                                TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].typeHint_get(), HorizontalAlignment.Left);
                                                                tb.FontStyle = FontStyle.Italic;
                                                                p.Children.Add(tb);
                                                            }
                                                        }
                                                        toReturn.Children.Add(p);
                                                        Grid.SetRow(p, 1);
                                                    }

                                                    {
                                                        ListView p = new ListView() {
                                                            HorizontalContentAlignment = HorizontalAlignment.Left,
                                                            VerticalContentAlignment = VerticalAlignment.Center,
                                                            ItemContainerStyle = cweeXamlHelper.StaticStyleResource("cweeListViewSimpleItemStyle"),
                                                            HorizontalAlignment = HorizontalAlignment.Left,
                                                            Padding = new Thickness(10, 0, 5, 0),
                                                            Margin = new Thickness(0),
                                                            MaxHeight = 100,
                                                            MaxWidth = 400,
                                                            BorderBrush = fgc.Result,
                                                            BorderThickness = new Thickness(1, 0, 0, 0)
                                                        };
                                                        EdmsTasks.InsertJob(() => {
                                                            var compatible_functions = vm?.ParentVM?.ParentVM?.engine?.DoScript_Cast_VectorStrings($"summarize_contained_functions({nodes[0].text_get()})");
                                                            if (compatible_functions != null && compatible_functions.Count > 0)
                                                            {
                                                                foreach (var compatible_function in compatible_functions)
                                                                {
                                                                    string temp = compatible_function;
                                                                    EdmsTasks.InsertJob(() => {
                                                                        var x = cweeXamlHelper.SimpleTextBlock(temp, HorizontalAlignment.Left);
                                                                        x.Margin = new Thickness(5, 0, 5, 0);
                                                                        x.FontSize = 14;
                                                                        p.Items.Add(x);
                                                                    }, true);
                                                                }

                                                                EdmsTasks.InsertJob(() =>
                                                                {
                                                                    {
                                                                        var p2 = cweeXamlHelper.SimpleTextBlock("Constructors", HorizontalAlignment.Left);
                                                                        p2.Margin = new Thickness(0, 5, 0, 0);
                                                                        toReturn.Children.Add(p2);
                                                                        Grid.SetRow(p2, 2);
                                                                    }
                                                                    toReturn.Children.Add(p);
                                                                    Grid.SetRow(p, 3);
                                                                }, true);
                                                            }
                                                        }, false);
                                                        
                                                    }
                                                    {                                                        
                                                        ListView p = new ListView() {
                                                            HorizontalContentAlignment = HorizontalAlignment.Left,
                                                            VerticalContentAlignment = VerticalAlignment.Center,
                                                            ItemContainerStyle = cweeXamlHelper.StaticStyleResource("cweeListViewSimpleItemStyle"),
                                                            HorizontalAlignment = HorizontalAlignment.Left,
                                                            Padding = new Thickness(10, 0, 5, 0),
                                                            Margin = new Thickness(0),
                                                            MaxHeight = 100,
                                                            MaxWidth = 400,
                                                            BorderBrush = fgc.Result,
                                                            BorderThickness = new Thickness(1, 0, 0, 0)
                                                        };
                                                        EdmsTasks.InsertJob(() => {
                                                            var compatible_functions = vm?.ParentVM?.ParentVM?.engine?.DoScript_Cast_VectorStrings($"summarize_compatible_functions({nodes[0].text_get()})");
                                                            if (compatible_functions != null && compatible_functions.Count > 0)
                                                            {
                                                                foreach (var compatible_function in compatible_functions)
                                                                {
                                                                    string temp = compatible_function;
                                                                    EdmsTasks.InsertJob(() => {
                                                                        var x = cweeXamlHelper.SimpleTextBlock(temp, HorizontalAlignment.Left);
                                                                        x.Margin = new Thickness(5, 0, 5, 0);
                                                                        x.FontSize = 14;
                                                                        p.Items.Add(x);
                                                                    }, true);
                                                                }

                                                                EdmsTasks.InsertJob(() =>
                                                                {
                                                                    {
                                                                        var p2 = cweeXamlHelper.SimpleTextBlock("Compatible Class Functions", HorizontalAlignment.Left);
                                                                        p2.Margin = new Thickness(0, 5, 0, 0);
                                                                        toReturn.Children.Add(p2);
                                                                        Grid.SetRow(p2, 4);
                                                                    }
                                                                    toReturn.Children.Add(p);
                                                                    Grid.SetRow(p, 5);
                                                                }, true);
                                                            }
                                                        }, false);
                                                    }
                                                }, true);
                                            }
                                            else
                                            {
                                                // function call
                                                fgc.ContinueWith(() => {
                                                    StackPanel p = new StackPanel() { Spacing = 5, Orientation = Orientation.Horizontal };
                                                    {
                                                        {
                                                            TextBlock tb = cweeXamlHelper.SimpleTextBlock("(Function)", HorizontalAlignment.Left);
                                                            p.Children.Add(tb);
                                                        }
                                                        {
                                                            SymbolIcon i = new SymbolIcon() { Symbol = Symbol.Library, Foreground = fgc.Result };
                                                            p.Children.Add(i);
                                                        }
                                                        if (!string.IsNullOrEmpty(nodes[0].typeHint_get()))
                                                        {
                                                            TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].typeHint_get(), HorizontalAlignment.Left);
                                                            tb.FontStyle = FontStyle.Italic;
                                                            p.Children.Add(tb);
                                                        }
                                                        {
                                                            TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].text_get(), HorizontalAlignment.Left);
                                                            p.Children.Add(tb);
                                                        }
                                                    }
                                                    toReturn.Children.Add(p);
                                                    Grid.SetRow(p, 1);

                                                    {
                                                        ListView p2 = new ListView() {
                                                            HorizontalContentAlignment = HorizontalAlignment.Left,
                                                            VerticalContentAlignment = VerticalAlignment.Center,
                                                            ItemContainerStyle = cweeXamlHelper.StaticStyleResource("cweeListViewSimpleItemStyle"),
                                                            HorizontalAlignment = HorizontalAlignment.Left,
                                                            Padding = new Thickness(10, 0, 5, 0),
                                                            Margin = new Thickness(0),
                                                            MaxHeight = 100,
                                                            MaxWidth = 400,
                                                            BorderBrush = fgc.Result,
                                                            BorderThickness = new Thickness(1, 0, 0, 0)
                                                        };
                                                        EdmsTasks.InsertJob(() => {
                                                            var compatible_functions = vm?.ParentVM?.ParentVM?.engine?.DoScript_Cast_VectorStrings($"summarize_contained_functions({nodes[0].text_get()})");
                                                            if (compatible_functions != null && compatible_functions.Count > 0)
                                                            {
                                                                foreach (var compatible_function in compatible_functions)
                                                                {
                                                                    string temp = compatible_function;
                                                                    EdmsTasks.InsertJob(() => {
                                                                        var x = cweeXamlHelper.SimpleTextBlock(temp, HorizontalAlignment.Left);
                                                                        x.Margin = new Thickness(5, 0, 5, 0);
                                                                        x.FontSize = 14;
                                                                        p2.Items.Add(x);
                                                                    }, true);
                                                                }

                                                                EdmsTasks.InsertJob(() =>
                                                                {
                                                                    {
                                                                        var p3 = cweeXamlHelper.SimpleTextBlock("Overloads", HorizontalAlignment.Left);
                                                                        p3.Margin = new Thickness(0,5,0,0);
                                                                        toReturn.Children.Add(p3);
                                                                        Grid.SetRow(p3, 2);
                                                                    }
                                                                    toReturn.Children.Add(p2);
                                                                    Grid.SetRow(p2, 3);
                                                                }, true);
                                                            }
                                                        }, false);
                                                    }
                                                }, true);
                                            }                 
                                            break;
                                    }
                                }
                                else
                                {
                                    fgc.ContinueWith(() => {
                                        StackPanel p = new StackPanel() { Spacing = 5, Orientation = Orientation.Horizontal };
                                        {
                                            {
                                                SymbolIcon i = new SymbolIcon() { Symbol = Symbol.Preview, Foreground = fgc.Result };
                                                p.Children.Add(i);
                                            }
                                            if (!string.IsNullOrEmpty(nodes[0].typeHint_get())) {
                                                TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].typeHint_get(), HorizontalAlignment.Left);
                                                tb.FontStyle = FontStyle.Italic;
                                                p.Children.Add(tb);
                                            }
                                            {
                                                TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].text_get(), HorizontalAlignment.Left);
                                                p.Children.Add(tb);
                                            }
                                        }
                                        toReturn.Children.Add(p);
                                        Grid.SetRow(p, 1);
                                    }, true);
                                }
                            }
                            break;

                        case WaterWatchEnums.ScriptNodeType.Error:
                            // an error
                            {
                                TextBlock tb = cweeXamlHelper.SimpleTextBlock(nodes[0].text_get(), HorizontalAlignment.Left);
                                toReturn.Children.Add(tb);
                                Grid.SetRow(tb, 1);
                            }
                            break;

                        case WaterWatchEnums.ScriptNodeType.Compiled:
                            // explain to the user that this has been compiled to C++
                            {
                                TextBlock tb = cweeXamlHelper.SimpleTextBlock("Compiled to C++ for performance. No debug information is available after compilation.", HorizontalAlignment.Left);
                                toReturn.Children.Add(tb);
                                Grid.SetRow(tb, 1);
                            }
                            break;
                    }
                }
                return toReturn;
            }
            return null;
        }



        internal static cweeDequeue dwellTimer;
        internal bool dwellPossible = false;
        internal string prevString = "";
        internal DateTime dwellCooldownTimer = new DateTime();
        private void Editor_DwellStart(Editor sender, DwellStartEventArgs args) {
            dwellTimer?.Cancel();

            if (Editor.ContextFlyout != null && Editor.ContextFlyout is cweeTipFlyout)
            {
                var fot = Editor.ContextFlyout as cweeTipFlyout;
                if (fot != null && fot.IsOpen)
                {
                    return;
                }
            }

            if (dwellPossible && args.Position >= 0)
            {
                dwellTimer = new cweeDequeue(DateTime.Now.AddSeconds(0.5), () => {
                    var x = FindNodesAtPosition(args.Position);
                    if (x != null && x.Count > 0)
                    {
                        if ((DateTime.Now - dwellCooldownTimer).TotalSeconds > 2) {
                            dwellCooldownTimer = DateTime.Now;
                            var flyout = Editor.SetFlyout(ReviewNodeList(x), Editor, Editor, new Point(sender.PointXFromPosition(args.Position), 24 + sender.PointYFromPosition(args.Position)), true);
                            flyout.LightDismissOverlayMode = LightDismissOverlayMode.Off;
                            flyout.ShowMode = FlyoutShowMode.TransientWithDismissOnPointerMoveAway;
                            flyout.Placement = FlyoutPlacementMode.BottomEdgeAlignedLeft;
                            flyout.AllowFocusOnInteraction = false;
                        }
                    }                   
                }, true);
            }
        }
        private void Editor_DwellEnd(Editor sender, DwellEndEventArgs args) {
            dwellTimer?.Cancel();
            dwellTimer = null;            
        }
        private void Tb_PointerEntered(object sender, PointerRoutedEventArgs e)
        {
            dwellPossible = true;

            dwellTimer?.Cancel();
            dwellTimer = null;
        }
        private void Tb_PointerExited(object sender, PointerRoutedEventArgs e) {
            dwellPossible = false;

            dwellTimer?.Cancel();
            dwellTimer = null;
        }


        private void Editor_IndicatorClick(Editor sender, IndicatorClickEventArgs args)
        {
            string warning = vm.errorManager.GetWarning(-1);
            if (!string.IsNullOrEmpty(warning))
            {
                var tb = cweeXamlHelper.SimpleTextBlock(warning);

                var flyout = Editor.SetFlyout(tb, Editor, Editor, new Point(sender.PointXFromPosition(args.Position), 24 + sender.PointYFromPosition(args.Position)), true);
                flyout.LightDismissOverlayMode = LightDismissOverlayMode.Off;
                flyout.ShowMode = FlyoutShowMode.TransientWithDismissOnPointerMoveAway;
                flyout.Placement = FlyoutPlacementMode.BottomEdgeAlignedLeft;
                flyout.AllowFocusOnInteraction = false;
            }

        }

        private void Editor_ZoomChanged(Editor sender, ZoomChangedEventArgs args)
        {
            if (sender.Zoom != defaultZoom)
            {
                sender.Zoom = defaultZoom;
            }
        }

        static public void UpdateCollapseMargin(MicaEditor.Editor sender)
        {
            System.Collections.Generic.Dictionary<long, int> parents = new System.Collections.Generic.Dictionary<long, int>();

            for (long line = sender.LineCount - 1; line >= 0; line--)
            {
                var parent_p = sender.GetFoldParent(line);
                if (parent_p >= 0)
                {
                    if (!parents.ContainsKey(parent_p))
                    {
                        parents.Add(parent_p, 0);
                    }
                }
                if (sender.MarginGetText(line) != "")
                {
                    sender.MarginSetText(line, "");
                }
            }

            foreach (var parent in parents.Keys)
            {
                if (sender.GetFoldExpanded(parent))
                {
                    sender.MarginSetText(parent, "⌐");
                }
                else
                {
                    sender.MarginSetText(parent, "≡");
                }
            }
        }



        public static int startOffsetForType(WaterWatchEnums.ScriptNodeType type, string line, int start)
        {
            // start = start - 1;
            if (start <= 0) { return 0; }
            if (start >= line.Length) { start = line.Length - 1; }

            string lineLeft = (start >= 1) ? line.Substring(0, start) : "";
            switch (type)
            {
                case WaterWatchEnums.ScriptNodeType.Id: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Unused_Return_Fun_Call: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Arg_List: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Arg: { return 0; }

                case WaterWatchEnums.ScriptNodeType.Array_Call: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Lambda: { var iF = lineLeft.FindLast("fun["); if (iF >= 0) { return lineLeft.Length - iF; } else { iF = lineLeft.FindLast("fun("); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } } }
                case WaterWatchEnums.ScriptNodeType.Block: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Scopeless_Block: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Def: { var iF = lineLeft.FindLast("def "); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }

                case WaterWatchEnums.ScriptNodeType.Constant: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Reference: { var iF = lineLeft.FindLast("var&"); if (iF >= 0) { return lineLeft.Length - iF; } else { iF = lineLeft.FindLast("auto&"); if (iF >= 0) { return lineLeft.Length - iF; } else { iF = lineLeft.FindLast("global&"); if (iF >= 0) { return lineLeft.Length - iF; } else { iF = lineLeft.FindLast("&"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } } } } }
                // case WaterWatchEnums.ScriptNodeType.Reference: { var iF = lineLeft.FindLast("&"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Attr_Decl: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Var_Decl: { var iF = lineLeft.FindLast("var"); if (iF >= 0) { return lineLeft.Length - iF; } else { iF = lineLeft.FindLast("auto"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } } }
                case WaterWatchEnums.ScriptNodeType.Assign_Decl: { var iF = lineLeft.FindLast("var"); if (iF >= 0) { return lineLeft.Length - iF; } else { iF = lineLeft.FindLast("auto"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } } }
                case WaterWatchEnums.ScriptNodeType.Global_Decl: { var iF = lineLeft.FindLast("global"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }

                case WaterWatchEnums.ScriptNodeType.Binary: { return 0; } // +, -, =, etc. 
                case WaterWatchEnums.ScriptNodeType.Method: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Dot_Access: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Fun_Call: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Logical_And: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Logical_Or: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Equation: { return 0; } // equals signs

                case WaterWatchEnums.ScriptNodeType.Inline_Array: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Inline_Map: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Return: { var iF = lineLeft.FindLast("return"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.File: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Prefix: { return 2; }

                case WaterWatchEnums.ScriptNodeType.Map_Pair: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Value_Range: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Inline_Range: { return 0; }

                case WaterWatchEnums.ScriptNodeType.If: { var iF = lineLeft.FindLast("if"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.While: { var iF = lineLeft.FindLast("while"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.For: { var iF = lineLeft.FindLast("for"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Ranged_For: { var iF = lineLeft.FindLast("for"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Break: { var iF = lineLeft.FindLast("break"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Continue: { var iF = lineLeft.FindLast("continue"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Do: { var iF = lineLeft.FindLast("do"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Try: { var iF = lineLeft.FindLast("try"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Catch: { var iF = lineLeft.FindLast("catch"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Finally: { var iF = lineLeft.FindLast("finally"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }

                case WaterWatchEnums.ScriptNodeType.Switch: { var iF = lineLeft.FindLast("switch"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Case: { var iF = lineLeft.FindLast("case"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }
                case WaterWatchEnums.ScriptNodeType.Default: { var iF = lineLeft.FindLast("default"); if (iF >= 0) { return lineLeft.Length - iF; } else { return 0; } }

                case WaterWatchEnums.ScriptNodeType.Class: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Compiled: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Error: { return 0; }
                case WaterWatchEnums.ScriptNodeType.Noop: { return 0; }

                default: return 0;
            }
        }

        public static Dictionary<string, cweeTask<SolidColorBrush>> scriptContentToColors = new Dictionary<string, cweeTask<SolidColorBrush>>() {
            { "Selection Highlight", cweeXamlHelper.ThemeColor("cweeDarkBlue") },
            // { "Basic Background", cweeXamlHelper.ThemeColor("cweeWhite") },
            { "Basic Foreground", cweeXamlHelper.ThemeColor("cweeDarkBlue") },
            { "Binary", cweeXamlHelper.ThemeColor("cweeBlue") },
            { "Error", cweeXamlHelper.ThemeColor("cweeRed") },
            { "Basic Function Names", cweeXamlHelper.ThemeColor("cweePurple") },
            { "Function Names", cweeXamlHelper.ThemeColor("cweePeach") },
            { "Object Names", cweeXamlHelper.ThemeColor("cweeStorage") },
            { "Strings", cweeXamlHelper.ThemeColor("cweeOrange") },
            { "Comments", cweeXamlHelper.ThemeColor("cweeGreen") },
            { "Compiled", cweeXamlHelper.ThemeColor("cweeFigureOverlay") },
        };
        public static List<cweeTask<SolidColorBrush>> VerticalBarColors = new List<cweeTask<SolidColorBrush>>() {
            cweeXamlHelper.ThemeColor("cweeDarkBlue"),
            cweeXamlHelper.ThemeColor("cweeDarkBlue").Blend(cweeXamlHelper.ThemeColor("cweeStorage"), 0.25f),
            cweeXamlHelper.ThemeColor("cweeDarkBlue").Blend(cweeXamlHelper.ThemeColor("cweeStorage"), 0.5f),
            cweeXamlHelper.ThemeColor("cweeDarkBlue").Blend(cweeXamlHelper.ThemeColor("cweeStorage"), 0.75f),
            cweeXamlHelper.ThemeColor("cweeStorage"),
            cweeXamlHelper.ThemeColor("cweeStorage").Blend(cweeXamlHelper.ThemeColor("cweeRed"), 0.25f),
            cweeXamlHelper.ThemeColor("cweeStorage").Blend(cweeXamlHelper.ThemeColor("cweeRed"), 0.50f),
            cweeXamlHelper.ThemeColor("cweeStorage").Blend(cweeXamlHelper.ThemeColor("cweeRed"), 0.75f),
            cweeXamlHelper.ThemeColor("cweeRed"),
            cweeXamlHelper.ThemeColor("cweeRed").Blend(cweeXamlHelper.ThemeColor("cweeWhite"), 0.25f),
            cweeXamlHelper.ThemeColor("cweeRed").Blend(cweeXamlHelper.ThemeColor("cweeWhite"), 0.50f),
            cweeXamlHelper.ThemeColor("cweeRed").Blend(cweeXamlHelper.ThemeColor("cweeWhite"), 0.75f),
            cweeXamlHelper.ThemeColor("cweeWhite")
        };
        private static int numVerticalBarColors => VerticalBarColors.Count;
        private static cweeTask<SolidColorBrush> BasicFunctionNamesColor = scriptContentToColors["Basic Function Names"];
        private static cweeTask<SolidColorBrush> FunctionNamesColor = scriptContentToColors["Function Names"];
        private static cweeTask<SolidColorBrush> StringsColor = scriptContentToColors["Strings"];
        private static cweeTask<SolidColorBrush> CommentsColor = scriptContentToColors["Comments"];
        private static cweeTask<SolidColorBrush> DynamicObjectNamesColor = scriptContentToColors["Object Names"];
        private static cweeTask<SolidColorBrush> BasicForeground = scriptContentToColors["Basic Foreground"];
        private static cweeTask<SolidColorBrush> DEFAULT = cweeXamlHelper.ThemeColor("cweeDarkBlue"); //  VerticalBarColors[n];
        public static cweeTask<SolidColorBrush> colorForType(WaterWatchEnums.ScriptNodeType type, int depth = -1)
        {
            // int n = Math.Min(numVerticalBarColors - 1, Math.Max(0, depth));

            switch (type)
            {
                case WaterWatchEnums.ScriptNodeType.Id: { return scriptContentToColors["Object Names"]; }

                case WaterWatchEnums.ScriptNodeType.Unused_Return_Fun_Call: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Arg_List: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Arg: { return DEFAULT; }

                case WaterWatchEnums.ScriptNodeType.Array_Call: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Lambda: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Block: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Scopeless_Block: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Def: { return DEFAULT; }

                case WaterWatchEnums.ScriptNodeType.Constant: { return scriptContentToColors["Binary"]; }
                case WaterWatchEnums.ScriptNodeType.Reference: { return scriptContentToColors["Binary"]; }
                case WaterWatchEnums.ScriptNodeType.Attr_Decl: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Var_Decl: { return scriptContentToColors["Binary"]; }

                case WaterWatchEnums.ScriptNodeType.Assign_Retroactively: { return scriptContentToColors["Binary"]; }
                case WaterWatchEnums.ScriptNodeType.Assign_Decl: { return scriptContentToColors["Binary"]; }
                case WaterWatchEnums.ScriptNodeType.Global_Decl: { return scriptContentToColors["Binary"]; }


                case WaterWatchEnums.ScriptNodeType.Binary: { return scriptContentToColors["Binary"]; } // +, -, =, etc. 
                case WaterWatchEnums.ScriptNodeType.Method: { return scriptContentToColors["Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Dot_Access: { return scriptContentToColors["Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Fun_Call: { return scriptContentToColors["Function Names"]; } // return scriptContentToColors["Function Names"];
                case WaterWatchEnums.ScriptNodeType.Logical_And: { return scriptContentToColors["Binary"]; }
                case WaterWatchEnums.ScriptNodeType.Logical_Or: { return scriptContentToColors["Binary"]; }
                case WaterWatchEnums.ScriptNodeType.Equation: { return scriptContentToColors["Binary"]; } // equals signs

                case WaterWatchEnums.ScriptNodeType.Inline_Array: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Inline_Map: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Return: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.File: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Prefix: { return scriptContentToColors["Binary"]; }
                case WaterWatchEnums.ScriptNodeType.Postfix: { return scriptContentToColors["Binary"]; }

                case WaterWatchEnums.ScriptNodeType.Map_Pair: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Value_Range: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Inline_Range: { return DEFAULT; }

                case WaterWatchEnums.ScriptNodeType.If: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.While: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.For: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Ranged_For: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Break: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Continue: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Do: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Try: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Catch: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Finally: { return scriptContentToColors["Basic Function Names"]; }

                case WaterWatchEnums.ScriptNodeType.Switch: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Case: { return scriptContentToColors["Basic Function Names"]; }
                case WaterWatchEnums.ScriptNodeType.Default: { return scriptContentToColors["Basic Function Names"]; }


                case WaterWatchEnums.ScriptNodeType.Class: { return DEFAULT; }
                case WaterWatchEnums.ScriptNodeType.Compiled: { return scriptContentToColors["Compiled"]; } // replaces a chunk (i.e. for loop) with compiled C++ code. 
                case WaterWatchEnums.ScriptNodeType.Error: { return scriptContentToColors["Error"]; }
                case WaterWatchEnums.ScriptNodeType.Noop: { return CommentsColor; ; }

                default: return DEFAULT;
            }     
        }
        internal int SumCharactersFromPreviousLines(List<string> textJ, int lineN, string delim) {
            int toReturn = 0;
            for (int i = 0; i < lineN && i < textJ.Count; i++)
            {
                toReturn += textJ[i].Length;
                toReturn += delim.Length;
            }
            return toReturn;
        }
        internal static cweeTask<SolidColorBrush> black = cweeXamlHelper.ThemeColor("cweeBlack");
        internal static cweeTask<Color> black_col = cweeXamlHelper.ThemeColorCol("cweeBlack");
        internal static cweeTask<SolidColorBrush> bg = cweeXamlHelper.ThemeColor("cweePageBackground");
        internal static cweeTask<Color> bg_col = cweeXamlHelper.ThemeColorCol("cweePageBackground");
        internal EdmsTasks.cweeTask HandleFormatting(string v, bool wasSuccessful = false, string SuccessfulOutputString = null) {
            // was this the result of the last UNDO or REDO ?
            bool uniqueChange = true;
            if (Undos.Count > 0) if (Undos.Last.Value == v) uniqueChange = false;
            if (Redos.Count > 0) if (Redos.First.Value == v) uniqueChange = false;
            if (uniqueChange) {
                Redos.Clear();
                Undos.AddLast(previousScript);
                if (Undos.Count > maxUndos) {
                    Undos.RemoveFirst();
                }
            }
            previousScript = v;

            List<string> textJ;
            if (true)
            {
                string copy = previousScript;
                copy = copy.Replace("\r", "\n");
                textJ = copy.Split("\n").ToList();
            }
            string delim = "\n";


            var toReturn = new EdmsTasks.cweeTask(()=> { }, false, true);

            queueTm = DateTime.Now.AddSeconds(0.25);
            formatDeq.Dequeue(queueTm, ()=> {
                if (vm != null && vm.ParentVM != null) {
                    string whatGotParsed = previousScript;
                    vector_scriptingnode list = vm?.ParentVM?.ParentVM?.engine?.PreParseScript(whatGotParsed);
                    {
                        bool earlyExit = (previousScript != whatGotParsed);
                        if (earlyExit) {
                            toReturn.QueueJob();
                            return;
                        }

                        bool WasWarning = false;
                        string warningMessage = "Error: Unknown Parsing Error";
                        // Fix the start/end columns and line numbers. Columns are converted to character positions -- lines are not used after this block.
                        foreach (var node in list)
                        {
                            if (node.type_get() == WaterWatchEnums.ScriptNodeType.Error) {
                                warningMessage = node.text_get();
                                vm.errorManager.AddWarning(-1, warningMessage, ref vm.ParentVM);
                                WasWarning = true;
                            }

                            int startLine = Math.Max(0, node.startLine_get());
                            int endLine = Math.Max(0, node.endLine_get());

                            node.startColumn_set(node.startColumn_get() - 1);
                            node.endColumn_set(node.endColumn_get() - 1);

                            int offset = startOffsetForType(node.type_get(), textJ[Math.Min(Math.Max(0, startLine), textJ.Count - 1)], node.startColumn_get());
                            node.startColumn_set(node.startColumn_get() - offset);
                            node.startColumn_set(node.startColumn_get() + SumCharactersFromPreviousLines(textJ, startLine, delim));
                            node.endColumn_set(node.endColumn_get() + SumCharactersFromPreviousLines(textJ, endLine, delim));

                            node.startLine_set(startLine);
                            node.endLine_set(endLine);

                            if (node.startColumn_get() < 0) node.startColumn_set(0);
                        }
                        
                        if (!WasWarning || vm.MostRecentParsedNodes == null)
                        {
                            vm.MostRecentParsedNodes = list;
                            vm.MostRecentParsedNodes_NoError = list;
                            vm.MostRecentParsedScript_NoError = whatGotParsed;

                            vm.errorManager.RemoveWarning(-1);
                            if (wasSuccessful) { vm.errorManager.RemoveWarning(-2); }                            

                            EdmsTasks.InsertJob(() => {
                                return DoFormattingActual(whatGotParsed, WasWarning, SuccessfulOutputString, wasSuccessful);
                            }, false).ContinueWith(() => {
                                toReturn.QueueJob();
                            }, false);
                        }
                        else
                        {
                            var L = vm.MostRecentParsedNodes;
                            list.InsertRange(0, L);
                            //for (int n = 0; n < L.Count; n--)
                            //{
                            //    var Node = L[n];
                            //    if (Node.type_get() != WaterWatchEnums.ScriptNodeType.Error)
                            //    {
                            //        list.Insert(n, Node);
                            //    }
                            //}
                            vm.MostRecentParsedNodes = list;

                            EdmsTasks.InsertJob(() => {
                                return DoFormattingActual(whatGotParsed, WasWarning, warningMessage, wasSuccessful);
                            }, false).ContinueWith(() => {
                                toReturn.QueueJob();
                            }, false);
                        }
                    }
                }
                else {
                    queueTm = DateTime.Now.AddSeconds(0.25);
                    formatDeq.Dequeue(queueTm);
                }
            }, false);

            return toReturn;
        }

        private EdmsTasks.cweeTask DoFormattingActual(string whatGotParsed, bool WasWarning, string SuccessfulOutputString, bool wasSuccessful)
        {
            if (DateTime.Now < queueTm) return EdmsTasks.cweeTask.CompletedTask(null);
            else {
                string newV = "";
                for (long i = 0; i < Editor.Editor.LineCount; i++)
                {
                    var line = Editor.Editor.GetLine(i);
                    newV = newV + line; // .AddToDelimiter(line, "\n");
                }
                newV = newV.Replace("\r", "\n");


                if (newV == whatGotParsed)
                {
                    if (DateTime.Now < queueTm) return EdmsTasks.cweeTask.CompletedTask(null);

                    Editor.Editor.IndicatorCurrent = 2;
                    Editor.Editor.IndicatorClearRange(0, newV.Length - 1);
                    Editor.Editor.IndicatorClearRange(0, newV.Length - 2);
                    Editor.Editor.IndicatorClearRange(0, newV.Length - 3);
                    Editor.Editor.IndicatorClearRange(0, newV.Length - 4);
                    Editor.Editor.IndicatorClearRange(0, newV.Length - 5);
                    Editor.Editor.IndicatorClearRange(0, newV.Length - 6);

                    if (!WasWarning && wasSuccessful) {
                        if (vm != null && vm.ParentVM != null) {
                            if (SuccessfulOutputString != null)
                                vm.ParentVM.outputPanel.vm.StatusString = SuccessfulOutputString;
                            else
                                vm.ParentVM.outputPanel.vm.StatusString = "Successful parse.";
                        }
                        vm.errorManager.RemoveWarning(-1);
                        vm.errorManager.RemoveWarning(-2);
                    }
                    else {
                        (bool found, int start, int end) = TryParseErrorMessage(
                            SuccessfulOutputString, 
                            newV.Split("\n", StringSplitOptions.None).ToList(),
                            "\n"
                            );

                        if (!wasSuccessful) {
                            vm.errorManager.AddWarning(-1, SuccessfulOutputString, ref vm.ParentVM);
                        }

                        if (found && start <= end)
                        {
                            if (vm != null && vm.ParentVM != null) {
                                if (SuccessfulOutputString != null)
                                    vm.ParentVM.outputPanel.vm.StatusString = SuccessfulOutputString;
                                else
                                    vm.ParentVM.outputPanel.vm.StatusString = "Successful parse.";
                            }

                            Editor.Editor.IndicatorCurrent = 2;
                            Editor.Editor.IndicatorFillRange(start, end - start);

                        }
                    }
                }
                else
                {
                    HandleFormatting(newV, wasSuccessful, SuccessfulOutputString);
                }
                return EdmsTasks.cweeTask.CompletedTask(null);
            }
        }

        private (bool, int, int) TryParseErrorMessage(string err, List<string> textJ, string delim)
        {
            List<(bool, int, int)> errorMentions = new List<(bool, int, int)>();

            string uniqueName = vm?.ParentVM?.uniqueName; // looking for this in the error message. If not found, then the error is from a C++ compiled code or another script object.
            if (err != null)
            {
                if (err.Find(uniqueName) >= 0)
                {
                    var splits = err.Split(uniqueName).ToList();
                    // everything before it fails to mention it
                    for (int i = 1; i < splits.Count; i++)
                    {
                        var potential_site = splits[i];
                        var copy = potential_site.Replace(")", " ").Replace("(", " ").Replace(",", " ").Replace(".", " ").Replace("  ", " ").Trim();
                        while (copy != potential_site)
                        {
                            potential_site = copy;
                            copy = copy.Replace("  ", " ").Trim();
                        }
                        var numberSplit = potential_site.Split(" ");
                        if (numberSplit.Length >= 2)
                        {
                            if (int.TryParse(numberSplit[0], out int whichLine) && int.TryParse(numberSplit[1], out int whichColumn))
                            {
                                whichLine -= 2;
                                whichColumn -= 1;

                                int startChar = Math.Max(0, (SumCharactersFromPreviousLines(textJ, whichLine, delim) + whichColumn) - 1);
                                int endChar = startChar + 2;

                                errorMentions.Add((true, startChar, endChar));

                                // return (true, startChar, endChar);
                            }
                        }
                    }

                    /* example: 
                    [
                    (true, 0, 10)
                    (true, 0, 6)
                    (true, 0, 1)
                    ]
                    */
                    {
                        int minStartChar = int.MaxValue;
                        int maxEndChar = 0;
                        foreach (var j in errorMentions)
                        {
                            if (j.Item2 < minStartChar) minStartChar = j.Item2;
                            if (j.Item3 > maxEndChar) maxEndChar = j.Item3;
                        }
                        return (true, minStartChar, maxEndChar);
                    }


                }
                else
                {
                    // there is an error but we don't know if it belongs to this script or not. Try and parse the old way?
                    if (err.Find(" at (") >= 0)
                    {
                        var start = err.Find(" at (");
                        var end = err.Find(")", start);
                        var location = err.Mid(start, end - start).Replace("at (", "").Replace(")", "").Replace(",", " ").Replace("  ", " ").Replace("  ", " ").Replace("  ", " ").Trim();
                        var line_column = location.Split(' ');

                        if (line_column.Length >= 2)
                        {
                            if (int.TryParse(line_column[line_column.Length - 2], out int whichLine) && int.TryParse(line_column[line_column.Length - 1], out int whichColumn))
                            {
                                --whichLine;
                                --whichColumn;

                                int startChar = Math.Max(0, (SumCharactersFromPreviousLines(textJ, whichLine, delim) + whichColumn) - 1);
                                int endChar = startChar + 2;

                                return (true, startChar, endChar);
                            }
                        }
                    }
                }
            }
            return (false, 0, 0);
        }
        private void DoTextChanging(bool forceUpdate = false, bool wasSuccessful = false, string SuccessfulOutputString = null) {
            string v = "";
            for (long i = 0; i < Editor.Editor.LineCount; i++)
            {
                var line = Editor.Editor.GetLine(i);
                v = v + line; // .AddToDelimiter(line, "\n");
            }
            v = v.Replace("\r", "\n");
            try
            {
                if (forceUpdate || (previousScript == null || previousScript != v))
                {
                    HandleFormatting(v, wasSuccessful, SuccessfulOutputString);
                }
            }
            catch (Exception) { }       
        }
        private void Run_Click(object sender, RoutedEventArgs e)
        {
            EdmsTasks.cweeTask announce;
            if (vm != null && vm.ParentVM != null)
            {
                announce = EdmsTasks.InsertJob(()=> { 
                    vm.ParentVM.outputPanel.vm.StatusString = "Running...";
                }, true);
            }
            else
            {
                announce = EdmsTasks.cweeTask.CompletedTask(null);
            }
            announce.ContinueWith(()=> {
                vm.errorManager.RemoveWarning(-2);
                var result = vm.ParentVM.RunAsync();
                result.ContinueWith(() =>
                {
                    ScriptingNodeResult res = result.Result as ScriptingNodeResult;
                    RunClickedEvent.InvokeEventAsync(this, res);

                    bool successful = false;
                    if (!string.IsNullOrEmpty(res.Error))
                    {
                        vm.errorManager.AddWarning(-2, res.Error.Split('\n')[0], ref vm.ParentVM);

                        EdmsTasks.InsertJob(() => {
                            DoTextChanging(true, successful, res.Error.Split('\n')[0]);
                        }, true, true);
                    }
                    else
                    {
                        successful = true;
                        var resultJob = res.QueryResult("");
                        resultJob.ContinueWith(() => {
                            if (vm != null && vm.ParentVM != null)
                            {
                                string v = resultJob.Result as string;
                                if (string.IsNullOrEmpty(v))
                                {
                                    vm.ParentVM.outputPanel.vm.StatusString = "Output: \"\"";
                                }
                                else
                                {
                                    if (v.Length < 128)
                                    {
                                        vm.ParentVM.outputPanel.vm.StatusString = "Output: \"" + v + "\"";
                                    }
                                    else
                                    {
                                        vm.ParentVM.outputPanel.vm.StatusString = "Output: \"" + v.Mid(0, 128) + "...";
                                    }
                                }
                                DoTextChanging(true, successful, vm.ParentVM.outputPanel.vm.StatusString);
                            }
                        }, true);
                    }


                }, false);
            }, true);    
        }
        internal Border GetQuickVisualization(string script) {
            Border ResultContainer = new Border();

            var NodeResult = new ScriptingNodeResult(vm.ParentVM, null, script);

            if (!string.IsNullOrEmpty(NodeResult.Error))
            {
                var getErrText = ScriptNode_VisualizerVM.StringContent(NodeResult.Error, true);
                getErrText.ContinueWith(() => {
                    if (ResultContainer != null)
                    {
                        ResultContainer.Child = getErrText.Result;
                    }
                }, true);
            }
            else
            {
                // result is valid.
                var getContent = ScriptNode_VisualizerVM.GetNodeContent(new ScriptNode_VisualizerVM.SharedNodeResult() { additionalParams = "", result = NodeResult });
                getContent.ContinueWith(() => {
                    if (ResultContainer != null)
                    {
                        ResultContainer.Child = getContent.Result;
                    }
                }, true);
            }

            return ResultContainer;
        }
        AtomicInt counter = new AtomicInt(0);
        internal void TrackPointer() {

        }

        private void Parse_Click(object sender, RoutedEventArgs e) {
            DoTextChanging(true);
        }

        private void Stop_Click(object sender, RoutedEventArgs e)
        {
            vm?.ParentVM?.ParentVM?.engine.StopCurrentScript();
        }
    }
}
