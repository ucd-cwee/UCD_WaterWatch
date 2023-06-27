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
        private vector_scriptingnode _MostRecentParsedNodes;

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
            _MostRecentParsedNodes = null;
        }
        
        public vector_scriptingnode MostRecentParsedNodes { get { return _MostRecentParsedNodes; } internal set { _MostRecentParsedNodes = value; } }
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
            return (a.FontStyle == FontStyle
                && Functions.ListIsEqual(a.BackgroundColor, BackgroundColor)
                && Functions.ListIsEqual(a.ForegroundColor, ForegroundColor)
                && a.Underline == Underline
                && a.Bold == Bold
                && a.Italic == Italic
            );
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
        private ITextParagraphFormat defaultParagraphFormat;
        private ITextCharacterFormat defaultCharacterFormat;
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

            Editor.FontSize = ThisFontSize;
            Editor.TextDocument.AlignmentIncludesTrailingWhitespace = false;
            defaultParagraphFormat = Editor.TextDocument.GetDefaultParagraphFormat();
            defaultParagraphFormat.PageBreakBefore = FormatEffect.Off;
            defaultParagraphFormat.Style = ParagraphStyle.None;
            defaultParagraphFormat.WidowControl = FormatEffect.Off;
            // defaultParagraphFormat.SetLineSpacing(LineSpacingRule.Exactly, (float)((double)(ThisFontSize + 3)));
            defaultParagraphFormat.SetLineSpacing(LineSpacingRule.Exactly, (float)((double)(ThisFontSize + 3)));
            Editor.TextDocument.SetDefaultParagraphFormat(defaultParagraphFormat);

            defaultCharacterFormat = Editor.TextDocument.GetDefaultCharacterFormat();
            var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");
            cdb.ContinueWith(()=> {
                defaultCharacterFormat.ForegroundColor = cdb.Result.Color;
            }, true);
            var cpb = cweeXamlHelper.ThemeColor("cweePageBackground");
            cpb.ContinueWith(() => {
                defaultCharacterFormat.BackgroundColor = cpb.Result.Color;
            }, true);
            defaultCharacterFormat.FontStyle = FontStyle.Normal;
            defaultCharacterFormat.Size = (float)ThisFontSize;
            Editor.TextDocument.SetDefaultCharacterFormat(defaultCharacterFormat);

            Editor.FontSize = ThisFontSize;
            Editor.FontFamily = new FontFamily("Segoe UI");
            Editor.FontStretch = FontStretch.Normal;
            Editor.ClipboardCopyFormat = RichEditClipboardFormat.PlainText;
            Editor.HorizontalTextAlignment = TextAlignment.Left;
            Editor.IsHandwritingViewEnabled = false;
            Editor.IsTextPredictionEnabled = false;
            Editor.IsTextScaleFactorEnabled = false;

            Editor.PreviewKeyDown += Editor_PreviewKeyDown;
            Editor.CharacterReceived += Editor_CharacterReceived; ;
            Editor.IsSpellCheckEnabled = false;

            Editor.Document.UndoLimit = 0;

            this.PointerEntered += ScriptNode_Editor_PointerEntered;
            this.PointerExited += ScriptNode_Editor_PointerExited;
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
                    Point p = new Point(-1,-1);
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
        public void Undo() {
            if (Undos.Count > 0)
            {
                Redos.AddLast(vm.ParentVM.Script);
                vm.ParentVM.Script = Undos.Last.Value;
                HandleFormatting(vm.ParentVM.Script);
                Undos.RemoveLast();
            }
        }
        public void Redo() {
            if (Redos.Count > 0)
            {
                //programmaticChange = true;
                Undos.AddLast(vm.ParentVM.Script);
                vm.ParentVM.Script = Redos.First.Value;
                HandleFormatting(vm.ParentVM.Script);
                Redos.RemoveFirst();
                //programmaticChange = false;
            }
        }

        private ScriptingNode FindNodeUnderCarot(int charNum) {
            List<ScriptingNode> foundNodes = new List<ScriptingNode>();
            if (vm.MostRecentParsedNodes != null && charNum >= 0) {

                    for (int i = vm.MostRecentParsedNodes.Count - 1; i >= 0; i--)
                    {
                        var node = vm.MostRecentParsedNodes[i];
                        if (node.startColumn <= charNum && node.endColumn >= charNum)
                        {
                            if (node.type == WaterWatchEnums.ScriptNodeType.File || node.type == WaterWatchEnums.ScriptNodeType.Noop) continue;
                            if (node.type == WaterWatchEnums.ScriptNodeType.Id) { foundNodes = new List<ScriptingNode>(); }
                            foundNodes.Add(node);
                        }
                    }

                    if (foundNodes.Count > 0)
                    {

                        return foundNodes[0];
                    }
                    else return null;
            }
            else
            {
                return null;
            }
        }

        class FunctionTipManager
        {
            public string AssociatedScriptNode = "";
            public List<string> orig_functions;

            private string _bestMatch = "";

            public string written = "";

            public ListView tips = new ListView() {
                AllowFocusOnInteraction = false,
                MaxHeight = 200,
                Margin = new Thickness(5)
            };

            private FrameworkElement CreateFunctionTip(string func) {
                return cweeXamlHelper.SimpleTextBlock(func, HorizontalAlignment.Left);
            }

            public void UpdateTips() {
                if (orig_functions != null)
                {
                    if (string.IsNullOrEmpty(written))
                    {
                        // display all tips
                        List<FrameworkElement> elements = new List<FrameworkElement>();
                        bool doOnce = true;
                        _bestMatch = "";
                        var ordered = orig_functions.OrderBy((string x)=> { string comp = x.ToLower(); if (comp.CompareTo("A") < 0) { comp = "z" + comp; } return comp; });
                        if (ordered != null)
                        {
                            foreach (var func in ordered)
                            {
                                if (doOnce) { _bestMatch = func; doOnce = false; };
                                elements.Add(CreateFunctionTip(func));
                            }
                        }
                        tips.ItemsSource = elements;
                    }
                    else
                    {
                        // sort tips based on whats written so far
                        List<FrameworkElement> elements = new List<FrameworkElement>();
                        bool doOnce = true;
                        _bestMatch = "";
                        var where = orig_functions.Where((string f) => { return f.StartsWith(written); });
                        if (where != null)
                        {
                            var ordered = where.OrderBy((string x) => { string comp = x.ToLower(); if (comp.CompareTo("A")<0) { comp = "z" + comp; } return comp; });
                            if (ordered != null)
                            {

                                foreach (var func in ordered)
                                {
                                    if (doOnce) { _bestMatch = func; doOnce = false; };
                                    elements.Add(CreateFunctionTip(func));
                                }
                            }
                        }
                        tips.ItemsSource = elements;
                    }
                }
            }

            public bool HasRecommendations() {
                return tips.Items.Count > 0;
            }

            public string GetCurrentMatch()
            {
                return _bestMatch;
            }

        }


        private void HideAllFunctionTips() {
            if (thisTip != null) {
                thisTip.IsOpen = false;
                thisTip.Target = null;
                if (
                    (vm.ParentVM.ParentVM.workspace != null) 
                    && 
                    (vm.ParentVM.ParentVM.workspace.vm.canvas.Children.Contains(thisTip))
                )
                {
                    vm.ParentVM.ParentVM.workspace.vm.canvas.Children.Remove(thisTip);
                }
            }
        }
        private FunctionTipManager GetOpenFunctionTipManager()
        {
            if (thisTip != null && thisTip.Tag is FunctionTipManager)
            {
                if (thisTip.IsOpen)
                {
                    return thisTip.Tag as FunctionTipManager;
                }
            }
            return null;
        }
        private TeachingTip CreateFunctionTip(UIElement content, FunctionTipManager ftm, string Title = null, string Subtitle = null) {
            thisTip.Tag = ftm;
            thisTip.Target = this; // .Editor;// was this
            thisTip.PreferredPlacement = TeachingTipPlacementMode.Bottom;
            thisTip.IsLightDismissEnabled = false;
            thisTip.AllowFocusOnInteraction = false;
            thisTip.HeroContent = content;
            
            if (!string.IsNullOrEmpty(Title)) { thisTip.Title = Title; }
            if (!string.IsNullOrEmpty(Subtitle)) { thisTip.Subtitle = Subtitle; }

            if (!vm.ParentVM.ParentVM.workspace.vm.canvas.Children.Contains(thisTip)) {
                vm.ParentVM.ParentVM.workspace.vm.canvas.Children.Add(thisTip);                
            }
            thisTip.Closed += (TeachingTip sender, TeachingTipClosedEventArgs args)=>{
                sender.Target = null;
            };
            thisTip.IsOpen = true;

            return thisTip;
        }

        private TeachingTip thisTip = new TeachingTip();
        // private AtomicInt thisTipLock = new AtomicInt(0);

        private void Editor_PreviewKeyDown(object sender, KeyRoutedEventArgs e)
        {
            var ftm = GetOpenFunctionTipManager();
            if (ftm != null && ftm.AssociatedScriptNode != vm.ParentVM.uniqueName)
            {
                HideAllFunctionTips();
            }

            int keyCode = (int)e.Key;
            try
            {
                RichEditBox tb = sender as RichEditBox;
                if (IsPressed(VirtualKey.Up)) {
                    var left = vm.ParentVM.Script.Left(tb.TextDocument.Selection.EndPosition);
                    int c = left.Replace("\r", "\n").Count('\n', left.Length);
                    if (c == 0)
                    {
                        e.Handled = true;
                        return;
                    }
                }
                if (IsPressed(VirtualKey.Down))
                {
                    var right = vm.ParentVM.Script.Right(vm.ParentVM.Script.Length - tb.TextDocument.Selection.EndPosition);
                    int c = right.Replace("\r", "\n").Count('\n', right.Length);
                    if (c == 0)
                    {
                        e.Handled = true;
                        return;
                    }
                }

                if (e.Key == VirtualKey.Delete || e.Key == VirtualKey.Back)
                {
                    HideAllFunctionTips();
                }
                if (e.Key == VirtualKey.Enter)
                {
                    if (ftm != null)
                    {
                        string toWrite = ftm.GetCurrentMatch().Right(ftm.GetCurrentMatch().Length - ftm.written.Length);
                        HideAllFunctionTips();
                        e.Handled = true;
                        tb.Document.Selection.TypeText(toWrite);
                        return;
                    }

                    e.Handled = true;
                    tb.Document.Selection.TypeText("\n");
                    return;
                }
                if (IsPressed(VirtualKey.Control))
                {
                    if (IsPressed(VirtualKey.Z))
                    {
                        HideAllFunctionTips();
                        Undo();
                        e.Handled = true;
                        return;
                    }
                    if (IsPressed(VirtualKey.Y))
                    {
                        HideAllFunctionTips();
                        Redo();
                        e.Handled = true;
                        return;
                    }
                    if (IsPressed(VirtualKey.V))
                    {
                        HideAllFunctionTips();
                        var f = Functions.GetTextFromClipboard();
                        f.ContinueWith(() =>
                        {
                            tb.Document.Selection.TypeText(f.Result);
                        }, true);
                        e.Handled = true;
                        return;
                    }
                    if (IsPressed(VirtualKey.X))
                    {
                        HideAllFunctionTips();
                        tb.Document.Selection.GetText(TextGetOptions.NoHidden, out string v);
                        tb.Document.Selection.TypeText("");

                        v = v.Replace("\r", "\n");
                        Functions.CopyToClipboard(v);
                        e.Handled = true;
                        return;
                    }
                    if (IsPressed(VirtualKey.C))
                    {
                        tb.Document.Selection.GetText(TextGetOptions.NoHidden, out string v);
                        v = v.Replace("\r", "\n");
                        Functions.CopyToClipboard(v);
                        e.Handled = true;
                        return;
                    }
                    return; // was not handled
                }
                if (IsPressed(VirtualKey.Shift))
                {
                    if (e.Key == VirtualKey.Tab)
                    {
                        HideAllFunctionTips();
                        // TO-DO, actually get the number of tabs in the highlighted region and undo the number of tabs per-line by one.
                        tb.Document.Selection.TypeText("\t");
                        e.Handled = true;
                        return;
                    }
                }
                if (e.Key == VirtualKey.Tab)
                {
                    HideAllFunctionTips();
                    // TO-DO, add a tab to the start of each line in the highlighted region. UNLESS only one line is selected, then insert a single tab like normal.
                    tb.Document.Selection.TypeText("\t");
                    e.Handled = true;
                    return;
                }
                if (e.Key == VirtualKey.Decimal || ((int)(0xBE) == keyCode)) {
                    // do we have an existing parse? 
                    if (vm.MostRecentParsedNodes != null) {
                        int position = tb.TextDocument.Selection.StartPosition;
                        var node = FindNodeUnderCarot(position);
                        if (node != null)
                        {
                            if (!string.IsNullOrEmpty(node.typeHint))
                            {
                                EdmsTasks.InsertJob(()=> { 
                                    List<string> funcs = vm.ParentVM.ParentVM.engine.CompatibleFunctions(node.typeHint);
                                    if (funcs.Count > 0) {
                                        EdmsTasks.InsertJob(() => {
                                            HideAllFunctionTips();
                                            ftm = new FunctionTipManager() { orig_functions = funcs, written = "", AssociatedScriptNode = vm.ParentVM.uniqueName };
                                            ftm.UpdateTips();
                                            var TeachingTip = CreateFunctionTip(ftm.tips, ftm, node.typeHint);
                                        }, true, true);
                                    }
                                }, false);
                            }
                            else if (!string.IsNullOrEmpty(node.text)) {
                                if (node.type == WaterWatchEnums.ScriptNodeType.Id)
                                {
                                    EdmsTasks.InsertJob(() => {
                                        List<string> funcs = vm.ParentVM.ParentVM.engine.CompatibleFunctions(node.text);

                                        if (funcs.Count > 0) {
                                            EdmsTasks.InsertJob(() => {
                                                HideAllFunctionTips();
                                                ftm = new FunctionTipManager() { orig_functions = funcs, written = "", AssociatedScriptNode = vm.ParentVM.uniqueName };
                                                ftm.UpdateTips();
                                                var TeachingTip = CreateFunctionTip(ftm.tips, ftm, node.text);
                                            }, true, true);
                                        }
                                    }, false);
                                }
                            }
                                    
                        }
                    }
                }
            }
            catch (Exception) { }
            finally
            {
                //prevScriptLock.Decrement();
            }
            
        }

        private void Editor_CharacterReceived(UIElement sender, CharacterReceivedRoutedEventArgs args)
        {
            try {
                var ftm = GetOpenFunctionTipManager();
                if (ftm != null)
                {
                    if (ftm.AssociatedScriptNode != vm.ParentVM.uniqueName || !args.Character.IsAlphaNumeric())
                    {
                        HideAllFunctionTips();
                    }
                    else 
                    {
                        ftm.written = ftm.written + args.Character;
                        ftm.UpdateTips();
                        if (!ftm.HasRecommendations())
                        {
                            HideAllFunctionTips();
                        }
                    }
                }
                else
                {
                    if (args.Character.IsAlphaNumeric())
                    {
                        string comparison = $"{args.Character}";
                        EdmsTasks.InsertJob(() =>
                        {
                            // potentially writing a new command
                            List<string> funcs = vm.ParentVM.ParentVM.engine.FunctionsThatStartWith(comparison);
                            if (funcs.Count > 0)
                            {
                                EdmsTasks.InsertJob(() =>
                                {
                                    ftm = new FunctionTipManager() { orig_functions = funcs, written = comparison, AssociatedScriptNode = vm.ParentVM.uniqueName };
                                    ftm.UpdateTips();
                                    var TeachingTip = CreateFunctionTip(ftm.tips, ftm);
                                }, true);
                            }
                        }, false);
                    }


                }
            }
            catch (Exception){ }
        }



        private void Editor_OnLoaded(object sender, RoutedEventArgs e)
        {
            RichEditBox tb = sender as RichEditBox;
            tb.Loaded -= Editor_OnLoaded;

            ScrollViewer myScroll = tb.FindVisualChild<ScrollViewer>();
            if (myScroll != null)
            {
                myScroll.BringIntoViewOnFocusChange = false;
                myScroll.AllowFocusOnInteraction = false;
                myScroll.VerticalScrollMode = ScrollMode.Disabled;
                myScroll.HorizontalScrollMode = ScrollMode.Disabled;
                myScroll.IsScrollInertiaEnabled = false;
                myScroll.BringIntoViewRequested += (UIElement sender2, BringIntoViewRequestedEventArgs args)=> { args.Handled = true; };

                myScroll.IsDeferredScrollingEnabled = false;
                myScroll.IsScrollInertiaEnabled = false;
                myScroll.IsVerticalScrollChainingEnabled = false;
                myScroll.ZoomMode = ZoomMode.Disabled;
                myScroll.ZoomSnapPointsType = SnapPointsType.None;
                myScroll.IsZoomChainingEnabled = false;
                myScroll.IsZoomInertiaEnabled = false;

                myScroll.CancelDirectManipulations();
            }
            DoTextChanging(true, true, "Created a new node.");
            
        }

        private Grid CreateLineNumber(int num) {
            var toReturn = new Grid() {
                Height = 20.0 + (ThisFontSize - 12.0) * 1.3, HorizontalAlignment = HorizontalAlignment.Stretch,  VerticalAlignment = VerticalAlignment.Stretch, Padding = new Thickness(0), Margin = new Thickness(0) };
            toReturn.Children.Add(new TextBlock() { 
                FontSize= ThisFontSize, Style = cweeXamlHelper.StaticStyleResource("cweeTextBlock"), Text = num.ToString(), Padding = new Thickness(0,0.6,0,0), Margin=new Thickness(0), HorizontalTextAlignment = TextAlignment.Center 
            });
            return toReturn;
        }
        private cweeTask<Grid> CreateLineOverlay(int num)
        {
            var cdb = cweeXamlHelper.ThemeColor("cweeDarkBlue");
            return cdb.ContinueWith(()=> {
                var toReturn = new Grid()
                {
                    Height = 20.0 + (ThisFontSize - 12.0) * 1.3,
                    BorderBrush = cdb.Result,
                    BorderThickness = new Thickness(0, 0, 0, 1),
                    Opacity = 0.2,
                    HorizontalAlignment = HorizontalAlignment.Stretch,
                    VerticalAlignment = VerticalAlignment.Stretch,
                    Padding = new Thickness(0),
                    Margin = new Thickness(0)
                };
                return toReturn;
            }, true);

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
        internal int SumCharactersFromPreviousLines(List<string> textJ, int lineN) {
            int toReturn = 0;
            for (int i = 0; i < lineN && i < textJ.Count; i++)
            {
                toReturn += textJ[i].Length;
                toReturn += 1;
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

                for (int i = LineNumbers.Children.Count; i < textJ.Count; i++)
                {
                    LineNumbers.Children.Add(CreateLineNumber(i + 1));
                    var overlay = CreateLineOverlay(i + 1);
                    overlay.ContinueWith(()=> {
                        LineOverlays.Children.Add(overlay.Result);
                    }, true);
                }
                for (int i = LineNumbers.Children.Count - 1; i >= textJ.Count; i--)
                {
                    LineNumbers.Children.RemoveAt(i);
                    LineOverlays.Children.RemoveAt(i);
                }
            }

            var toReturn = new EdmsTasks.cweeTask(()=> { }, false, true);

            queueTm = DateTime.Now.AddSeconds(0.25);
            formatDeq.Dequeue(queueTm, ()=> {
                if (vm != null && vm.ParentVM != null)
                {
                    string whatGotParsed = previousScript;
                    vector_scriptingnode list = vm?.ParentVM?.ParentVM?.engine?.PreParseScript(whatGotParsed);
                    {
                        //prevScriptLock.Lock();
                        bool earlyExit = (previousScript != whatGotParsed);
                        //prevScriptLock.Unlock();
                        if (earlyExit)
                        {
                            toReturn.QueueJob();

                            return;
                        }

                        List<FormatCharDetails> charColors = new List<FormatCharDetails>(whatGotParsed.Length);

                        // Fix the start/end columns and line numbers. Columns are converted to character positions -- lines are not used after this block.
                        foreach (var node in list)
                        {
                            int startLine = Math.Max(0, node.startLine);
                            int endLine = Math.Max(0, node.endLine);

                            node.startColumn = node.startColumn - 1;
                            node.endColumn = node.endColumn - 1;

                            int offset = startOffsetForType(node.type, textJ[Math.Min(Math.Max(0, startLine), textJ.Count - 1)], node.startColumn);
                            node.startColumn -= offset;
                            node.startColumn += SumCharactersFromPreviousLines(textJ, startLine);
                            node.endColumn += SumCharactersFromPreviousLines(textJ, endLine);

                            node.startLine = startLine;
                            node.endLine = endLine;

                            if (node.startColumn < 0) node.startColumn = 0;
                        }
                        {
                            for (int i = 0; i < whatGotParsed.Length; i++)
                            {
                                var x = new FormatCharDetails();
                                {
                                    x.Bold = FormatEffect.Off;
                                    x.FontStyle = FontStyle.Normal;
                                    x.Italic = FormatEffect.Off;
                                    x.Underline = UnderlineType.None;
                                    x.BackgroundColor.Add(bg);
                                    x.ForegroundColor.Add(BasicForeground);
                                }
                                charColors.Add(x);
                            }
                        }
                        bool WasWarning = false;

                        foreach (var node in list)
                        {
                            if (node.type == WaterWatchEnums.ScriptNodeType.Error)
                            {
                                WasWarning = true;
                                break;
                            }
                        }

                        if (!WasWarning || vm.MostRecentParsedNodes == null)
                        {
                            vm.MostRecentParsedNodes = list;
                            vm.errorManager.RemoveWarning(-1);
                            if (wasSuccessful) { vm.errorManager.RemoveWarning(-2); }
                        }
                        else
                        {
                            for (int n = vm.MostRecentParsedNodes.Count - 1; n >= 0; n--)
                            {
                                var Node = vm.MostRecentParsedNodes[n];
                                if (Node.type == WaterWatchEnums.ScriptNodeType.Error)
                                {
                                    vm.MostRecentParsedNodes.RemoveAt(n);
                                }
                            }
                            vm.MostRecentParsedNodes.AddRange(list);
                            list = vm.MostRecentParsedNodes;
                        }

                        List<EdmsTasks.cweeTask> jobs = new List<EdmsTasks.cweeTask>();
                        foreach (var node2 in list)
                        {
                            var node = node2;
                            if (node.type == WaterWatchEnums.ScriptNodeType.Error)
                            {
                                vm.errorManager.AddWarning(-1, node.text.Split('\n')[0], ref vm.ParentVM);

                                WasWarning = true;

                                continue;
                            }
                            else
                            {
                                cweeTask<SolidColorBrush> brush;
                                // unique coloring based on the node function
                                brush = colorForType(node2.type, node.depth);
                                switch (node2.type) {
                                    // do nothing in terms of formatting
                                    case WaterWatchEnums.ScriptNodeType.File:
                                    case WaterWatchEnums.ScriptNodeType.Block:
                                    case WaterWatchEnums.ScriptNodeType.Scopeless_Block: break; 
                                    // comments
                                    case WaterWatchEnums.ScriptNodeType.Noop: {
                                            jobs.Add(brush.ContinueWith(() =>
                                            {
                                                for (int i = node.startColumn; i < node.endColumn && i < charColors.Count; i++)
                                                {
                                                    charColors[i].ForegroundColor.Add(brush);
                                                    charColors[i].Underline = UnderlineType.None;
                                                    charColors[i].Italic = FormatEffect.On;
                                                }
                                            }, false));
                                            break;
                                        }
                                    // Mutex-lock that all threads must capture to progress
                                    case WaterWatchEnums.ScriptNodeType.ControlBlock: {
                                            jobs.Add(brush.ContinueWith(() =>
                                            {
                                                for (int i = node.startColumn; i < node.endColumn && i < charColors.Count; i++)
                                                {
                                                    charColors[i].Italic = FormatEffect.On;
                                                }
                                            }, false));
                                            break;
                                        }
                                    // constants built-in to the script (i.e. doubles, strings, etc.)
                                    case WaterWatchEnums.ScriptNodeType.Constant: {
                                            jobs.Add(brush.ContinueWith(() =>
                                            {
                                                for (int i = node.startColumn; i < node.endColumn && i < charColors.Count; i++)
                                                {
                                                    charColors[i].ForegroundColor.Add(brush);
                                                    charColors[i].Underline = UnderlineType.Single;
                                                }
                                            }, false));
                                            break;
                                        }
                                    case WaterWatchEnums.ScriptNodeType.Error: {
                                            jobs.Add(brush.ContinueWith(() =>
                                            {
                                                for (int i = node.startColumn; i < node.endColumn && i < charColors.Count; i++)
                                                {
                                                    charColors[i].ForegroundColor = new List<cweeSolidColorBrush>() { brush };
                                                    charColors[i].Underline = UnderlineType.HeavyWave;
                                                }
                                            }, false));
                                            break;
                                        }
                                    case WaterWatchEnums.ScriptNodeType.Id: 
                                    case WaterWatchEnums.ScriptNodeType.Fun_Call: 
                                    case WaterWatchEnums.ScriptNodeType.Unused_Return_Fun_Call: 
                                    case WaterWatchEnums.ScriptNodeType.Arg_List: 
                                    case WaterWatchEnums.ScriptNodeType.Equation: 
                                    case WaterWatchEnums.ScriptNodeType.Var_Decl: 
                                    case WaterWatchEnums.ScriptNodeType.Assign_Decl: 
                                    case WaterWatchEnums.ScriptNodeType.Array_Call: 
                                    case WaterWatchEnums.ScriptNodeType.Dot_Access: 
                                    case WaterWatchEnums.ScriptNodeType.Lambda: 
                                    case WaterWatchEnums.ScriptNodeType.Def: 
                                    case WaterWatchEnums.ScriptNodeType.While: 
                                    case WaterWatchEnums.ScriptNodeType.If: 
                                    case WaterWatchEnums.ScriptNodeType.For: 
                                    case WaterWatchEnums.ScriptNodeType.Ranged_For: 
                                    case WaterWatchEnums.ScriptNodeType.Inline_Array: 
                                    case WaterWatchEnums.ScriptNodeType.Inline_Map: 
                                    case WaterWatchEnums.ScriptNodeType.Return: 
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
                                    case WaterWatchEnums.ScriptNodeType.Class: 
                                    case WaterWatchEnums.ScriptNodeType.Binary: 
                                    case WaterWatchEnums.ScriptNodeType.Arg: 
                                    case WaterWatchEnums.ScriptNodeType.Global_Decl:
                                    case WaterWatchEnums.ScriptNodeType.Compiled:
                                    case WaterWatchEnums.ScriptNodeType.Postfix: 
                                    case WaterWatchEnums.ScriptNodeType.Assign_Retroactively:                                     
                                    default: {
                                            jobs.Add(brush.ContinueWith(() =>
                                            {
                                                for (int i = node.startColumn; i < node.endColumn && i < charColors.Count; i++)
                                                {
                                                    charColors[i].ForegroundColor.Add(brush);
                                                }
                                            }, false));
                                            break;
                                        }
                                }

                                // unique coloring based on the return type of the node
                                if (!string.IsNullOrEmpty(node.typeHint))
                                {
                                    if (node.typeHint == "string" || node.typeHint == "cweeStr")
                                    {
                                        brush = scriptContentToColors["Strings"];
                                        jobs.Add(brush.ContinueWith(() =>
                                        {
                                            for (int i = node.startColumn; i < node.endColumn && i < charColors.Count; i++)
                                            {
                                                charColors[i].ForegroundColor.Add(brush);
                                            }
                                        }, false));
                                    }
                                }

                                // background shading to indicate script depth
                                if (true) {
                                    var brushJob = bg.Blend(black, 1.0f - Math.Min(1.0f, Math.Max(0.01f, node.depth / 10.0f)));
                                    jobs.Add(brushJob.ContinueWith(() =>
                                    {
                                        for (int i = node.startColumn; i < node.endColumn && i < charColors.Count; i++)
                                        {
                                            charColors[i].BackgroundColor.Add(brushJob);
                                        }
                                    }, false));
                                }
                            }
                        }

                        EdmsTasks.cweeTask.TrueWhenCompleted(jobs).ContinueWith(() =>
                        {
                            jobs.Clear();
                            var brush = colorForType(WaterWatchEnums.ScriptNodeType.Error);
                            foreach (int errCode in vm.errorManager.Errors.Keys.ToList())
                            {
                                if (vm.errorManager.Errors.TryGetValue(errCode, out CodeError err))
                                {
                                    // if (err.ParentVM == this.vm.ParentVM)
                                    {
                                        var ErrReply = TryParseErrorMessage(err.Error, textJ);
                                        if (ErrReply.Item1)
                                        {
                                            for (int i = ErrReply.Item2; i < ErrReply.Item3 && i < charColors.Count; i++)
                                            {
                                                charColors[i].BackgroundColor.Add(bg);
                                                charColors[i].ForegroundColor.Clear();
                                                charColors[i].ForegroundColor.Add(brush);
                                                charColors[i].Underline = UnderlineType.HeavyWave;
                                                charColors[i].FontStyle = FontStyle.Italic;
                                                charColors[i].Bold = FormatEffect.On;
                                                charColors[i].Italic = FormatEffect.Off;
                                            }
                                        }
                                    }
                                }
                            }

                            List<FormatStrDetails> finalFormat = new List<FormatStrDetails>();
                            {
                                FormatCharDetails prev = null;
                                int prevStart = 0;
                                for (int i = 0; i < charColors.Count; i++)
                                {
                                    if (prev == null)
                                    {
                                        prevStart = i;
                                        prev = charColors[i];
                                        continue;
                                    }

                                    if (prev != charColors[i])
                                    {
                                        finalFormat.Add(new FormatStrDetails(prev, prevStart, i));
                                        prev = charColors[i];
                                        prevStart = i;
                                        continue;
                                    }
                                }
                                if (prev != null)
                                {
                                    finalFormat.Add(new FormatStrDetails(prev, prevStart, charColors.Count));
                                }
                            }

                            return bg_col.ContinueWith(() =>
                            {
                                Color bgc = bg_col.Result;
                                foreach (var formatter2 in finalFormat)
                                {
                                    var formatter = formatter2;
                                    var abg = cweeSolidColorBrush.AvgColor(formatter.details.BackgroundColor).Lerp(bgc, 0.85f);
                                    jobs.Add(abg.ContinueWith(() =>
                                    {
                                        formatter.details.AvgBackgroundColor = abg.Result;
                                        var afg = cweeSolidColorBrush.StratifyColor(formatter.details.ForegroundColor).Lerp(formatter.details.AvgBackgroundColor.GetBestContrastingColor(), 0.01f);
                                        return afg.ContinueWith(() =>
                                        {
                                            formatter.details.AvgForegroundColor = afg.Result;
                                        }, false);
                                    }, false));

                                    //var afg = cweeSolidColorBrush.StratifyColor(formatter.details.ForegroundColor).Lerp(formatter.details.AvgBackgroundColor.GetBestContrastingColor(), 0.1f);
                                    //jobs.Add(afg.ContinueWith(() =>
                                    //{
                                    //    formatter.details.AvgForegroundColor = afg.Result;
                                    //}, false));
                                }
                                return EdmsTasks.cweeTask.TrueWhenCompleted(jobs).ContinueWith(() => {
                                    return DoFormattingActual(whatGotParsed, finalFormat, WasWarning, SuccessfulOutputString, wasSuccessful);
                                }, true);
                            }, false);
                        }, false).ContinueWith(()=> {
                            toReturn.QueueJob();
                        }, false);
                    }
                }
                else
                {
                    queueTm = DateTime.Now.AddSeconds(0.25);
                    formatDeq.Dequeue(queueTm);
                }
            }, false);

            return toReturn;
        }        
        private EdmsTasks.cweeTask DoFormattingActual(string whatGotParsed, List<FormatStrDetails> finalFormat, bool WasWarning, string SuccessfulOutputString, bool wasSuccessful)
        {
            if (DateTime.Now < queueTm) return EdmsTasks.cweeTask.CompletedTask(null);
            else {
                Editor.TextDocument.GetText(TextGetOptions.NoHidden, out string newV);
                newV = newV.Replace("\r", "\n");
                if (newV == whatGotParsed)
                {
                    if (true)
                    {
                        var sel = Editor.TextDocument.Selection;
                        var startPos = sel.StartPosition;
                        var endPos = sel.EndPosition;

                        EditorParent.Children.Remove(Editor);

                        sel.SetRange(0, startPos); // newV.Length + 1);
                        sel.ScrollIntoView(PointOptions.NoVerticalScroll);
                        sel.ParagraphFormat = defaultParagraphFormat;
                        sel.CharacterFormat = defaultCharacterFormat;
                        sel.SetRange(startPos, whatGotParsed.Length);
                        sel.ScrollIntoView(PointOptions.Start);
                        sel.ParagraphFormat = defaultParagraphFormat;
                        sel.CharacterFormat = defaultCharacterFormat;

                        foreach (var formatter in finalFormat)
                        {
                            if (DateTime.Now < queueTm) break;

                            sel.SetRange(formatter.start, formatter.end);
                            sel.CharacterFormat.ForegroundColor = formatter.details.AvgForegroundColor;
                            sel.CharacterFormat.BackgroundColor = formatter.details.AvgBackgroundColor;
                            if (sel.CharacterFormat.Bold != formatter.details.Bold) sel.CharacterFormat.Bold = formatter.details.Bold;
                            if (sel.CharacterFormat.FontStyle != formatter.details.FontStyle) sel.CharacterFormat.FontStyle = formatter.details.FontStyle;
                            if (sel.CharacterFormat.Italic != formatter.details.Italic) sel.CharacterFormat.Italic = formatter.details.Italic;
                            if (sel.CharacterFormat.Underline != formatter.details.Underline) sel.CharacterFormat.Underline = formatter.details.Underline;
                        }

                        //sel.SetRange(startPos, endPos);
                        EditorParent.Children.Add(Editor);
                        Editor.Focus(FocusState.Programmatic);

                        sel.SetRange(startPos, endPos);
                        sel.ScrollIntoView(PointOptions.Start);
                    }

                    if (DateTime.Now < queueTm) return EdmsTasks.cweeTask.CompletedTask(null);

                    if (!WasWarning)
                    {
                        if (vm != null && vm.ParentVM != null)
                        {
                            if (SuccessfulOutputString != null)
                                vm.ParentVM.outputPanel.vm.StatusString = SuccessfulOutputString;
                            else
                                vm.ParentVM.outputPanel.vm.StatusString = "Successful parse.";
                        }
                        vm.errorManager.RemoveWarning(-1);
                        vm.errorManager.RemoveWarning(-2);

                    }
                }
                else
                {
                    HandleFormatting(newV, wasSuccessful, SuccessfulOutputString);
                }
                return EdmsTasks.cweeTask.CompletedTask(null);
            }
        }
        private (bool, int, int) TryParseErrorMessage(string err, List<string> textJ)
        {
            List<(bool, int, int)> errorMentions = new List<(bool, int, int)>();

            string uniqueName = vm?.ParentVM?.uniqueName; // looking for this in the error message. If not found, then the error is from a C++ compiled code or another script object.
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

                            int startChar = Math.Max(0, (SumCharactersFromPreviousLines(textJ, whichLine) + whichColumn) - 1);
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

                            int startChar = Math.Max(0, (SumCharactersFromPreviousLines(textJ, whichLine) + whichColumn) - 1);
                            int endChar = startChar + 2;

                            return (true, startChar, endChar);
                        }
                    }
                }
            }

            return (false, 0, 0);
        }
        private void DoTextChanging(bool forceUpdate = false, bool wasSuccessful = false, string SuccessfulOutputString = null) {
            Editor.TextDocument.GetText(TextGetOptions.NoHidden, out string v);
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
        private void Editor_OnTextChanged(object sender, RoutedEventArgs e)
        {
            DoTextChanging();
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
        internal void TrackPointer()
        {
            if (ScriptNode_EditorVM.pointerPosition != prevPosition)
            {
                counter.Set(0);
                prevPosition = ScriptNode_EditorVM.pointerPosition;
            }
            else
            {
                if (counter.Increment() == 2) {
                    if (trackPointerLock.TryIncrementTo(1))
                    {
                        var shouldR = EdmsTasks.InsertJob(() => { return Editor.ProofingMenuFlyout.IsOpen || Editor.SelectionFlyout.IsOpen; }, true, true);
                        shouldR.ContinueWith(() =>
                        {
                            bool sr = shouldR.Result;
                            if (!sr)
                            {

                                var args = AppExtension.ApplicationInfo.PointerMovedArgs;//  = args;
                                if (vm.MostRecentParsedNodes != null && args != null)
                                {
                                    EdmsTasks.InsertJob(() => {
                                        var position = args.GetCurrentPoint(Editor).Position;
                                        if ((position.X >= 0 && position.X <= Editor.ActualWidth) && (position.Y >= 0 && position.Y <= Editor.ActualHeight))
                                        {
                                            return EdmsTasks.InsertJob(() =>
                                            {
                                                var range = Editor.Document.GetRangeFromPoint(position, PointOptions.ClientCoordinates);
                                                int charNum = range.StartPosition;
                                                {
                                                    List<ScriptingNode> foundNodes = new List<ScriptingNode>();
                                                    return EdmsTasks.InsertJob(() =>
                                                    {
                                                        for (int i = vm.MostRecentParsedNodes.Count - 1; i >= 0; i--)
                                                        {
                                                            var node = vm.MostRecentParsedNodes[i];
                                                            if (node.startColumn <= charNum && node.endColumn >= charNum)
                                                            {
                                                                if (node.type == WaterWatchEnums.ScriptNodeType.File || node.type == WaterWatchEnums.ScriptNodeType.Noop) continue;

                                                                foundNodes.Add(node);
                                                            }
                                                        }
                                                    }, true).ContinueWith(() =>
                                                    {
                                                        if (foundNodes.Count > 0)
                                                        {
                                                            var tt = new StackPanel() { Orientation = Orientation.Vertical };
                                                            var row1 = new BreadcrumbBar();
                                                            var row2 = new StackPanel() { Orientation = Orientation.Vertical, HorizontalAlignment = HorizontalAlignment.Left };
                                                            tt.Children.Add(row1);
                                                            tt.Children.Add(row2);

                                                            return EdmsTasks.InsertJob(() =>
                                                            {
                                                                List<EdmsTasks.cweeTask> tasks = new List<EdmsTasks.cweeTask>();
                                                                List<string> BreadcrumbBarContent = new List<string>();
                                                                foreach (var node in foundNodes)
                                                                {
                                                                    if (node.type == WaterWatchEnums.ScriptNodeType.Id) { BreadcrumbBarContent = new List<string>(); }
                                                                    BreadcrumbBarContent.Add(node.type.ToString());
                                                                }
                                                                BreadcrumbBarContent.Reverse();
                                                                tasks.Add(new EdmsTasks.cweeTask(() => { row1.ItemsSource = BreadcrumbBarContent; }, true, true));

                                                                // accum the flyout
                                                                tasks.Add(new EdmsTasks.cweeTask(() =>
                                                                {
                                                                    if (!string.IsNullOrEmpty(foundNodes[0].typeHint))
                                                                    {
                                                                        row2.Children.Add(cweeXamlHelper.SimpleTextBlock(foundNodes[0].typeHint, HorizontalAlignment.Left));
                                                                    }

                                                                    if ((foundNodes.Count > 0) && !string.IsNullOrEmpty(foundNodes[0].text))
                                                                    {
                                                                        if (!string.IsNullOrEmpty(foundNodes[0].typeHint))
                                                                        {
                                                                            switch (foundNodes[0].typeHint)
                                                                            {
                                                                                default:
                                                                                case "Object":
                                                                                    {
                                                                                        StackPanel header = new StackPanel() { Orientation = Orientation.Horizontal, Spacing = 10 };
                                                                                        header.Children.Add(new FontIcon() { FontFamily = new FontFamily("Segoe MDL2 Assets"), Glyph = "\xE8A5" });
                                                                                        header.Children.Add(cweeXamlHelper.SimpleTextBlock(foundNodes[0].typeHint, HorizontalAlignment.Left));
                                                                                        header.Children.Add(cweeXamlHelper.SimpleTextBlock(foundNodes[0].text, HorizontalAlignment.Left));
                                                                                        row2.Children.Add(header);


                                                                                        Border ResultContainer = GetQuickVisualization(foundNodes[0].text);
                                                                                        row2.Children.Add(ResultContainer);

                                                                                        break;
                                                                                    }
                                                                                case "int":
                                                                                case "double":
                                                                                case "float":
                                                                                    {
                                                                                        Border ResultContainer = GetQuickVisualization(foundNodes[0].text);
                                                                                        row2.Children.Add(ResultContainer);

                                                                                        break;
                                                                                    }
                                                                                case "Function":
                                                                                    {
                                                                                        StackPanel header = new StackPanel() { Orientation = Orientation.Horizontal, Spacing = 10 };
                                                                                        header.Children.Add(new FontIcon() { FontFamily = new FontFamily("Segoe MDL2 Assets"), Glyph = "\xE97B" });
                                                                                        header.Children.Add(cweeXamlHelper.SimpleTextBlock(foundNodes[0].text, HorizontalAlignment.Left));
                                                                                        row2.Children.Add(header);

                                                                                        string toGetVectorOfParams =
                                                                                        "{\n" +
                                                                                        $"    var& out = Vector();\n" +
                                                                                        $"    var& funcs = Vector({foundNodes[0].text}.get_contained_functions());\n" +
                                                                                        "    if (funcs.size == 0) {" +
                                                                                        $"       funcs.push_back_ref({foundNodes[0].text});\n" +
                                                                                        "    }" +
                                                                                        "    for (func : funcs)\n" +
                                                                                        "    {\n" +
                                                                                        "        var& funcInfo := func.get()\n" +
                                                                                        "        cweeStr& params = \"\";\n" +
                                                                                        "        if (funcInfo.contains(\"params\")){\n" +
                                                                                        "            for (param : funcInfo[\"params\"]){\n" +
                                                                                        "                params.AddToDelimiter(param.to_string(), \", \");\n" +
                                                                                        "            }\n" +
                                                                                        "        }\n" +
                                                                                        "        out.push_back_ref(\"${ funcInfo[\"returns\"] } = " + $"{foundNodes[0].text}" + "(${params});\");\n" +
                                                                                        "    }\n" +
                                                                                        "    return out;\n" +
                                                                                        "}";

                                                                                        var vec = vm.ParentVM.ParentVM.engine.DoScript_Cast_VectorStrings(toGetVectorOfParams);
                                                                                        foreach (var funcT in vec)
                                                                                        {
                                                                                            var funcT_text = funcT;
                                                                                            EdmsTasks.InsertJob(() =>
                                                                                            {
                                                                                                row2.Children.Add(cweeXamlHelper.SimpleTextBlock(funcT, HorizontalAlignment.Left));
                                                                                            }, true);
                                                                                        }
                                                                                        break;
                                                                                    }
                                                                                case "string":
                                                                                    {
                                                                                        Border ResultContainer = GetQuickVisualization("\"" + foundNodes[0].text + "\"");
                                                                                        row2.Children.Add(ResultContainer);
                                                                                        break;
                                                                                    }
                                                                            }
                                                                        }
                                                                        else
                                                                        {
                                                                            Border ResultContainer = GetQuickVisualization(foundNodes[0].text);
                                                                            row2.Children.Add(ResultContainer);
                                                                        }
                                                                    }
                                                                }, true, true));

                                                                return EdmsTasks.cweeTask.InsertListAsTask(tasks, false);
                                                            }, false).ContinueWith(() =>
                                                            {
                                                                // set the flyout
                                                                if (Editor.ContextFlyout == null || !Editor.ContextFlyout.IsOpen)
                                                                {
                                                                    var flyout = Editor.SetFlyout(tt, null, Editor, position.Add(new Point(10, 10)), true);
                                                                    flyout.LightDismissOverlayMode = LightDismissOverlayMode.Auto;
                                                                    flyout.ShowMode = FlyoutShowMode.TransientWithDismissOnPointerMoveAway;
                                                                    flyout.Tag = foundNodes[0].type;
                                                                    flyout.Closed += (object sender, object e) =>
                                                                    {
                                                                        Editor.ContextFlyout = null;
                                                                    };
                                                                }
                                                            }, true);
                                                        }
                                                        else return null;
                                                    }, true);
                                                }
                                            }, true);
                                        }
                                        else
                                        {
                                            return null;
                                        }
                                    }, true).ContinueWith(() => {
                                        trackPointerLock.Decrement();
                                    }, false);
                                }
                                else
                                {
                                    trackPointerLock.Decrement();
                                }
                            }
                            else
                            {
                                trackPointerLock.Decrement();
                            }
                        }, false);
                    }
                }
            }
        }

        private void ScrollViewer_BringIntoViewRequested(UIElement sender, BringIntoViewRequestedEventArgs args) {
            args.Handled = true;
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
