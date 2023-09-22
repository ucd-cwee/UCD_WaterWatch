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

using Microsoft.UI.Xaml.Controls;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using UWP_WaterWatch.Pages;
using UWP_WaterWatchLibrary;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Animation;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace UWP_WaterWatch.Custom_Controls
{
    internal static class Extensions
    {
        public static ScriptingPage GetScriptingPage(this TabViewItem tab)
        {
            if (tab != null && tab.Content is ScriptingPage)
            {
                return tab.Content as ScriptingPage;
            }
            return null;
        }
        public static TextScriptingPage GetTextScriptingPage(this TabViewItem tab)
        {
            if (tab != null && tab.Content is TextScriptingPage)
            {
                return tab.Content as TextScriptingPage;
            }
            return null;
        }
    }

    public class ScriptingManagerCommandBarViewModel : ViewModelBase
    {
        public TabView tabview;

        public ScriptingManagerCommandBarViewModel() { }
        ~ScriptingManagerCommandBarViewModel() {
            tabview = null;
        }

        public static cweeTask<(ScriptNode, Panel)> PrepareScriptNode(string UniqueName, ScriptingPage scriptPage)
        {
            var col = cweeXamlHelper.ThemeColor("cweeDarkBlue");
            return col.ContinueWith(()=> {
                if (scriptPage != null)
                {
                    var workSpacePanel = scriptPage.VM.workspace?.vm?.canvas;
                    if (workSpacePanel != null)
                    {
                        List<Windows.UI.Xaml.Shapes.Path> paths = new List<Windows.UI.Xaml.Shapes.Path>();
                        for (int i = 0; i < 10; i++)
                        {
                            paths.Add(new Windows.UI.Xaml.Shapes.Path()
                            {
                                Visibility = Visibility.Collapsed,
                                Stroke = col.Result,
                                StrokeThickness = 3,
                                StrokeEndLineCap = PenLineCap.Flat,
                                StrokeStartLineCap = PenLineCap.Round
                            });
                        }

                        var newScriptNode = new ScriptNode(scriptPage.VM.outputPanel, paths, scriptPage, UniqueName);
                        {
                            // newScriptNode.vm.ParentVM = scriptPage.VM;
                            newScriptNode.vm.Script = "";
                            newScriptNode.vm.Label = "New Script Node";
                            newScriptNode.vm.DeleteSelfEvent += (object s, ScriptNode n) => {                               
                                EdmsTasks.InsertJob(() => {
                                    (n.Parent as Panel).Children.Remove(n);
                                }, true, true);
                            };
                            
                            newScriptNode.Loaded += (object sender, RoutedEventArgs e)=> {
                                ScriptNode n = sender as ScriptNode;
                                n.vm.ParentVM = scriptPage.VM;
                            };
                            newScriptNode.Unloaded += (object sender, RoutedEventArgs e) => {
                                ScriptNode n = sender as ScriptNode;
                                n.vm.ParentVM = null;
                            };
                        }

                        // try to center the node
                        var scrollviewer = scriptPage.VM.workspace?.vm?.scrollViewer;
                        if (scrollviewer != null)
                        {
                            newScriptNode.floatingControl.InitialPositionLeft =
                                $"{ ((scrollviewer.HorizontalOffset + (scrollviewer.ActualWidth / 2.0) - 50) / scrollviewer.ZoomFactor) / workSpacePanel.ActualWidth }*";
                            newScriptNode.floatingControl.InitialPositionTop =
                                $"{ ((scrollviewer.VerticalOffset + (scrollviewer.ActualHeight / 2.0) - 50) / scrollviewer.ZoomFactor) / workSpacePanel.ActualHeight }*";
                        }

                        foreach (var path in paths)
                        {
                            workSpacePanel.Children.Add(path);
                        }

                        return (newScriptNode, workSpacePanel);
                    }
                }
                return (null, null);
            }, true);
        }

        public static cweeTask<ScriptNode> AddScriptNode(string UniqueName, ScriptingPage scriptPage) {
            if (scriptPage != null)
            {
                var x = PrepareScriptNode(UniqueName, scriptPage);
                return x.ContinueWith(() =>
                {
                    x.Result.Item2.Children.Add(x.Result.Item1);
                    return x.Result.Item1;
                }, true);
            }
            else
            {
                return null;
            }
        }

        public class JSON_Serializer_Workspace
        {
            public List<string> nodes = new List<string>();
            public double canvasWidth = 0;
            public double canvasHeight = 0;
            public string ScriptName = "";
        }

        internal JSON_Serializer_Workspace SaveWorkspace()
        {
            var tosave = new JSON_Serializer_Workspace();
            {
                var scriptPage = GetCurrentScriptingPage();
                if (scriptPage != null)
                {
                    List<string> uniqueNodeNames = new List<string>();
                    if (scriptPage != null)
                    {
                        var workSpacePanel = scriptPage.VM.workspace?.vm?.canvas;
                        if (workSpacePanel != null)
                        {
                            foreach (var child in workSpacePanel.Children)
                            {
                                if (child is ScriptNode)
                                {
                                    ScriptNode node = child as ScriptNode;
                                    uniqueNodeNames.Add(node.vm.uniqueName);
                                    string json = node.ToJson();

                                    tosave.nodes.Add(json);
                                }
                            }
                        }
                        tosave.canvasWidth = scriptPage.VM.workspace.vm.canvasWidth;
                        tosave.canvasHeight = scriptPage.VM.workspace.vm.canvasHeight;
                    }

                    // Node deletion is async and unpredictable -- using the same names during loading after saving and deleting nodes may (is fast enough) cause bugs where nodes are deleted quickly after being made.
                    // Fix is to re-name nodes CONSISTENTLY.     
                    var newNodeNameConversions = new Dictionary<string, string>();
                    foreach (var prevName in uniqueNodeNames)
                    {
                        newNodeNameConversions[prevName] = ScriptingManagerCommandBar.NewScriptNodeName();
                        for (int i = 0; i < tosave.nodes.Count; i++)
                        {
                            tosave.nodes[i] = tosave.nodes[i].Replace(prevName, newNodeNameConversions[prevName]);
                        }
                    }

                    tosave.ScriptName = scriptPage.VM.ScriptName;
                }
            }
            {
                var scriptPage = GetCurrentTextScriptingPage();
                if (scriptPage != null)
                {
                    List<string> uniqueNodeNames = new List<string>();
                    if (scriptPage != null)
                    {
                        uniqueNodeNames.Add(scriptPage.thisNodeVM.uniqueName);

                        string json = scriptPage.ToJson();
                        tosave.nodes.Add(json);

                        tosave.canvasWidth = 0;
                        tosave.canvasHeight = 0;
                    }

                    // Node deletion is async and unpredictable -- using the same names during loading after saving and deleting nodes may (is fast enough) cause bugs where nodes are deleted quickly after being made.
                    // Fix is to re-name nodes CONSISTENTLY.     
                    var newNodeNameConversions = new Dictionary<string, string>();
                    foreach (var prevName in uniqueNodeNames)
                    {
                        newNodeNameConversions[prevName] = ScriptingManagerCommandBar.NewScriptNodeName();
                        for (int i = 0; i < tosave.nodes.Count; i++)
                        {
                            tosave.nodes[i] = tosave.nodes[i].Replace(prevName, newNodeNameConversions[prevName]);
                        }
                    }

                    tosave.ScriptName = scriptPage.VM.ScriptName;
                }
            }
            return tosave;
        }        
        internal static EdmsTasks.cweeTask ClearWorkspace(ScriptingPage scriptPage)
        {
            if (scriptPage != null && scriptPage.VM != null && scriptPage.VM.workspace != null)
            {
                return scriptPage.VM.workspace.ClearWorkspace();
            }
            else
            {
                return EdmsTasks.cweeTask.CompletedTask(null);
            }
        }
        internal static EdmsTasks.cweeTask ClearWorkspace(TextScriptingPage scriptPage)
        {
            if (scriptPage != null && scriptPage.VM != null)
            {
                return EdmsTasks.InsertJob(()=> {
                    scriptPage.thisNodeVM.Script = "";
                }, true);
            }
            else
            {
                return EdmsTasks.cweeTask.CompletedTask(null);
            }
        }

        private static void Node_ContinueAfterAllLoaded(object sender, RoutedEventArgs e)
        {
            ScriptNode node = sender as ScriptNode;
            node.Loaded -= Node_ContinueAfterAllLoaded;

            (string, List<ScriptingNodeViewModel>, List<ScriptNode>, EdmsTasks.cweeTask) t = ((string, List<ScriptingNodeViewModel>, List<ScriptNode>, EdmsTasks.cweeTask))node.Tag;

            node.vm.PositionChanged.InvokeEventAsync(node, false);

            t.Item4.SetFinished(null);
        }

        private static void NodeContinueLoading(object sender, RoutedEventArgs e)
        {
            ScriptNode node = sender as ScriptNode;
            EdmsTasks.InsertJob(()=> {
                (string, List<ScriptingNodeViewModel>, List<ScriptNode>, EdmsTasks.cweeTask) t = ((string, List<ScriptingNodeViewModel>, List<ScriptNode>, EdmsTasks.cweeTask))node.Tag;
                node.FromJson_p2(t.Item1, t.Item2, t.Item3);
                node.Tag = null;
                node.vm.PositionChanged.InvokeEventAsync(node, false);
            }, true, true);
        }

        public ScriptingPage GetCurrentScriptingPage()
        {
            return GetCurrentView().GetScriptingPage();
        }
        public TextScriptingPage GetCurrentTextScriptingPage()
        {
            return GetCurrentView().GetTextScriptingPage();
        }
        public TabViewItem GetCurrentView()
        {
            if (tabview != null && tabview.SelectedItem is TabViewItem)
            {
                return tabview.SelectedItem as TabViewItem;
            }
            return null;
        }
       
        private bool _ShowVisualTab = false;
        public bool ShowVisualTab { get { return _ShowVisualTab; } set { _ShowVisualTab = value; _ShowEditTab = _ShowTextTab || _ShowVisualTab;  OnPropertyChanged("ShowEditTab"); OnPropertyChanged("ShowVisualTab"); } }

        private bool _ShowTextTab = false;
        public bool ShowTextTab { get { return _ShowTextTab; } set { _ShowTextTab = value; _ShowEditTab = _ShowTextTab || _ShowVisualTab; OnPropertyChanged("ShowEditTab");  OnPropertyChanged("ShowTextTab"); } }

        private bool _ShowEditTab = false;
        public bool ShowEditTab { get { return _ShowEditTab; } set { _ShowEditTab = value; OnPropertyChanged("ShowEditTab"); } }


        internal void SaveScript()
        {
            {
                var page = GetCurrentScriptingPage();
                if (page != null)
                {
                    if (string.IsNullOrEmpty(page.VM.ScriptSavePath))
                    {
                        // Save as instead
                        SaveScriptAs();
                    }
                    else
                    {
                        string content = JsonConvert.SerializeObject(SaveWorkspace());

                        // save the script in the current path
                        {
                            {
                                SharedString str = new SharedString();
                                str.Set(content);
                                var tempFile = EdmsTasks.InsertJob(() => { return page.VM.engine.DoScript("try{" + $"var fp = createRandomFile(\"DAT\"); writeFileFromCweeStr(fp, external_data.GetString({str.Index()})); return fp;" + "}"); }, false);
                                // copy the file over
                                tempFile.ContinueWith(() =>
                                {
                                    var file_to_replace = RecallFile(page.VM.ScriptSavePath);
                                    file_to_replace.ContinueWith(() =>
                                    {
                                        if (file_to_replace.Result == null)
                                        {
                                            EdmsTasks.InsertJob(() =>
                                            {
                                                EdmsTasks.InsertJob(() =>
                                                {
                                                    SaveScriptAs();
                                                }, true, true);
                                            }, false, true);
                                            return;
                                        }

                                        AppExtension.ApplicationInfo.MainWindowAction(() =>
                                        {
                                            try
                                            {
                                                Windows.Storage.StorageFile.GetFileFromPathAsync(tempFile.Result as string).AsTask().ContinueWith((System.Threading.Tasks.Task<Windows.Storage.StorageFile> vf) =>
                                                {
                                                    try
                                                    {
                                                        AppExtension.ApplicationInfo.MainWindowAction(async () =>
                                                        {
                                                            await vf.Result.CopyAndReplaceAsync(file_to_replace.Result);
                                                        });
                                                    }
                                                    catch (Exception) { }
                                                });
                                            }
                                            catch (Exception) { }
                                        });
                                    }, false);
                                }, false);

                            }
                        }
                    }
                }
            }
            { 
                var page = GetCurrentTextScriptingPage();
                if (page != null)
                {
                    if (string.IsNullOrEmpty(page.VM.ScriptSavePath))
                    {
                        // Save as instead
                        SaveScriptAs();
                    }
                    else
                    {
                        string content = JsonConvert.SerializeObject(SaveWorkspace());

                        // save the script in the current path
                        {
                            {
                                SharedString str = new SharedString();
                                str.Set(content);
                                var tempFile = EdmsTasks.InsertJob(() => { return page.VM.engine.DoScript("try{" + $"var fp = createRandomFile(\"DAT\"); writeFileFromCweeStr(fp, external_data.GetString({str.Index()})); return fp;" + "}"); }, false);
                                // copy the file over
                                tempFile.ContinueWith(() =>
                                {
                                    var file_to_replace = RecallFile(page.VM.ScriptSavePath);
                                    file_to_replace.ContinueWith(() =>
                                    {
                                        if (file_to_replace.Result == null)
                                        {
                                            EdmsTasks.InsertJob(() =>
                                            {
                                                EdmsTasks.InsertJob(() =>
                                                {
                                                    SaveScriptAs();
                                                }, true, true);
                                            }, false, true);
                                            return;
                                        }

                                        AppExtension.ApplicationInfo.MainWindowAction(() =>
                                        {
                                            try
                                            {
                                                Windows.Storage.StorageFile.GetFileFromPathAsync(tempFile.Result as string).AsTask().ContinueWith((System.Threading.Tasks.Task<Windows.Storage.StorageFile> vf) =>
                                                {
                                                    try
                                                    {
                                                        AppExtension.ApplicationInfo.MainWindowAction(async () =>
                                                        {
                                                            await vf.Result.CopyAndReplaceAsync(file_to_replace.Result);
                                                        });
                                                    }
                                                    catch (Exception) { }
                                                });
                                            }
                                            catch (Exception) { }
                                        });
                                    }, false);
                                }, false);

                            }
                        }
                    }
                }
            }

        }

        internal cweeTask<string> RememberFile(Windows.Storage.StorageFile file, string path)
        {
            string attemptedCode = path.Replace(" ", "").Replace(":", "").Replace("\\", "").Replace("/", "").Replace(".", "");

            return EdmsTasks.InsertJob(()=> { 
                Windows.Storage.AccessCache.StorageApplicationPermissions.FutureAccessList.AddOrReplace(attemptedCode, file);
                return file.Path;
            }, true, true);
        }
        public cweeTask<Windows.Storage.StorageFile> RecallFile(string path)
        {
            string attemptedCode = path.Replace(" ", "").Replace(":", "").Replace("\\", "").Replace("/", "").Replace(".", "");
            return (EdmsTasks.cweeTask)EdmsTasks.InsertJob(()=> {
                if (Windows.Storage.AccessCache.StorageApplicationPermissions.FutureAccessList.ContainsItem(attemptedCode)) 
                {
                    EdmsTasks.cweeTask toReturn = new EdmsTasks.cweeTask();
                    EdmsTasks.InsertJob(() => {
                        AppExtension.ApplicationInfo.MainWindowAction(async () =>
                        {
                            var j = await Windows.Storage.AccessCache.StorageApplicationPermissions.FutureAccessList.GetFileAsync(attemptedCode);
                            toReturn.SetFinished(j);
                        });
                    }, false, true);                    
                    return toReturn;
                }
                return null;
            }, true, true);
        }

        internal void SaveScriptAs()
        {
            try
            {
                var page = GetCurrentScriptingPage();
                if (page != null)
                {
                    EdmsTasks.cweeTask fileJob = new EdmsTasks.cweeTask();

                    EdmsTasks.InsertJob(() => {
                        AppExtension.ApplicationInfo.MainWindowAction(async () =>
                        {
                            var savePicker = new Windows.Storage.Pickers.FileSavePicker();
                            savePicker.SuggestedFileName = page.VM.ScriptName;
                            savePicker.SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.DocumentsLibrary;
                            savePicker.FileTypeChoices["Scripts"] = new List<string>() { ".dat", ".txt" };

                            Windows.Storage.StorageFile file = await savePicker.PickSaveFileAsync();

                            fileJob.SetFinished(file);
                        });
                    }, false, true);

                    fileJob.ContinueWith(()=> {
                        if (fileJob.Result == null) { return; }
                        cweeTask<string> j = RememberFile(fileJob.Result, fileJob.Result.Path);
                        j.ContinueWith(()=> {
                            if (j.Result != null)
                            {
                                page.VM.ScriptSavePath = fileJob.Result.Path;
                                SaveScript();
                            }
                        }, true);
                    }, true);
                }
            }
            catch (Exception) { }

            try
            {
                var page = GetCurrentTextScriptingPage();
                if (page != null)
                {
                    EdmsTasks.cweeTask fileJob = new EdmsTasks.cweeTask();

                    EdmsTasks.InsertJob(() => {
                        AppExtension.ApplicationInfo.MainWindowAction(async () =>
                        {
                            var savePicker = new Windows.Storage.Pickers.FileSavePicker();
                            savePicker.SuggestedFileName = page.VM.ScriptName;
                            savePicker.SuggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.DocumentsLibrary;
                            savePicker.FileTypeChoices["Scripts"] = new List<string>() { ".script" };

                            Windows.Storage.StorageFile file = await savePicker.PickSaveFileAsync();

                            fileJob.SetFinished(file);
                        });
                    }, false, true);

                    fileJob.ContinueWith(() => {
                        if (fileJob.Result == null) { return; }
                        cweeTask<string> j = RememberFile(fileJob.Result, fileJob.Result.Path);
                        j.ContinueWith(() => {
                            if (j.Result != null)
                            {
                                page.VM.ScriptSavePath = fileJob.Result.Path;
                                SaveScript();
                            }
                        }, true);
                    }, true);
                }
            }
            catch (Exception) { }
        }

        internal void SaveAll()
        {
            // for each open script, perform the save or save as operation

            throw new NotImplementedException();
        }

        internal static void LoadWorkspace(JSON_Serializer_Workspace toload, ScriptingPage scriptPage)
        {
            ClearWorkspace(scriptPage).ContinueWith(()=> {
                if (toload != null)
                {
                    // change the names of everything...
                    {
                        var newNodeNameConversions = new Dictionary<string, string>();
                        foreach (var node_json in toload.nodes)
                        {
                            var n = Newtonsoft.Json.JsonConvert.DeserializeObject<ScriptingNodeViewModel.JSON_Serializer>(node_json);
                            newNodeNameConversions[n.uniqueName] = ScriptingManagerCommandBar.NewScriptNodeName();
                        }
                        foreach (var nameConv in newNodeNameConversions)
                        {
                            for (int i = 0; i < toload.nodes.Count; i++)
                            {
                                toload.nodes[i] = toload.nodes[i].Replace(nameConv.Key, nameConv.Value);
                            }
                        }
                    }

                    if (scriptPage != null)
                    {
                        scriptPage.VM.ScriptName = toload.ScriptName;
                        scriptPage.VM.workspace.vm.canvasWidth = toload.canvasWidth;
                        scriptPage.VM.workspace.vm.canvasHeight = toload.canvasHeight;

                        var workSpacePanel = scriptPage.VM.workspace?.vm?.canvas;

                        if (workSpacePanel.IsLoaded)
                        {
                            ContinueLoadingWorkspace(toload, scriptPage);
                        }
                        else
                        {
                            workSpacePanel.Tag = (toload, scriptPage);
                            workSpacePanel.Loaded += WorkSpacePanel_Loaded;
                        }
                    }
                }
            }, true);
        }
        internal static void LoadWorkspace(JSON_Serializer_Workspace toload, TextScriptingPage scriptPage)
        {
            ClearWorkspace(scriptPage).ContinueWith(() => {
                if (toload != null)
                {
                    // change the names of everything...
                    {
                        var newNodeNameConversions = new Dictionary<string, string>();
                        foreach (var node_json in toload.nodes)
                        {
                            var n = Newtonsoft.Json.JsonConvert.DeserializeObject<ScriptingNodeViewModel.JSON_Serializer>(node_json);
                            newNodeNameConversions[n.uniqueName] = ScriptingManagerCommandBar.NewScriptNodeName();
                        }
                        foreach (var nameConv in newNodeNameConversions)
                        {
                            for (int i = 0; i < toload.nodes.Count; i++)
                            {
                                toload.nodes[i] = toload.nodes[i].Replace(nameConv.Key, nameConv.Value);
                            }
                        }
                    }

                    if (scriptPage != null)
                    {
                        scriptPage.VM.ScriptName = toload.ScriptName;
                        scriptPage.VM.workspace.vm.canvasWidth = toload.canvasWidth;
                        scriptPage.VM.workspace.vm.canvasHeight = toload.canvasHeight;

                        foreach (var node_json in toload.nodes)
                        {
                            var node = Newtonsoft.Json.JsonConvert.DeserializeObject<ScriptingNodeViewModel.JSON_Serializer>(node_json);
                            scriptPage.thisNodeVM.Script = node.script;
                        }
                    }
                }
            }, true);
        }

        private static void WorkSpacePanel_Loaded(object sender, RoutedEventArgs e)
        {
            Panel workSpacePanel = sender as Panel;
            workSpacePanel.Loaded -= WorkSpacePanel_Loaded;
            (JSON_Serializer_Workspace, ScriptingPage) v = ((JSON_Serializer_Workspace, ScriptingPage))(workSpacePanel.Tag);
            workSpacePanel.Tag = null;
            ContinueLoadingWorkspace(v.Item1, v.Item2);
        }

        internal static void ContinueLoadingWorkspace(JSON_Serializer_Workspace toload, ScriptingPage scriptPage)
        {
            List<EdmsTasks.cweeTask> tasks = new List<EdmsTasks.cweeTask>();
            List<EdmsTasks.cweeTask> AfterLoadedTasks = new List<EdmsTasks.cweeTask>();

            List<ScriptNode> nodes = new List<ScriptNode>();
            List<ScriptingNodeViewModel> node_vms = new List<ScriptingNodeViewModel>();

            List<(ScriptNode, Panel)> todos = new List<(ScriptNode, Panel)>();

            foreach (var node_json in toload.nodes)
            {
#if true
                string uniqueName = Newtonsoft.Json.JsonConvert.DeserializeObject<ScriptingNodeViewModel.JSON_Serializer>(node_json).uniqueName;
                var N = PrepareScriptNode(uniqueName, scriptPage);
                tasks.Add(N.ContinueWith(()=> {
                    var node = N.Result.Item1 as ScriptNode;
                    nodes.Add(node);
                    node_vms.Add(node.vm);
                    node.FromJson_p1(node_json);
#if true
                    var awaitTask = new EdmsTasks.cweeTask();
                    node.Tag = (node_json, node_vms, nodes, awaitTask);
                    node.Loaded += Node_ContinueAfterAllLoaded;
                    // node.Loaded += NodeContinueLoading; // DO THE FOLLOW UP ATER _ALL_ NODES WERE LOADED
                    // node.Loaded 
#endif
                    todos.Add(N.Result);
                    AfterLoadedTasks.Add(awaitTask);
#endif
                }, true));
            }
            EdmsTasks.cweeTask.TrueWhenCompleted(tasks).ContinueWith(()=> {
                foreach (var x in todos)
                {
                    x.Item2.Children.Add(x.Item1);
                }
            }, true);
            EdmsTasks.cweeTask.TrueWhenCompleted(AfterLoadedTasks).ContinueWith(() => {
                foreach (var node in nodes)
                {
                    NodeContinueLoading(node, null);
                }
            }, true);


        }

        internal void OpenExistingScripts()
        {
            try {
                EdmsTasks.cweeTask fileJob = new EdmsTasks.cweeTask();
                EdmsTasks.InsertJob(() => {
                    AppExtension.ApplicationInfo.MainWindowAction(async () =>
                    {
                        var picker = new Windows.Storage.Pickers.FileOpenPicker();
                        picker.FileTypeFilter.Add(".dat");
                        picker.FileTypeFilter.Add(".txt");
                        picker.FileTypeFilter.Add(".script");
                        List< Windows.Storage.StorageFile> storageFiles = (await picker.PickMultipleFilesAsync()).ToList();

                        fileJob.SetFinished(storageFiles);
                    });
                }, false, true);

                fileJob.ContinueWith(() => {
                    List<Windows.Storage.StorageFile> storageFiles = fileJob.Result;
                    foreach (Windows.Storage.StorageFile file2 in storageFiles) {
                        Windows.Storage.StorageFile file = file2;
                        RememberFile(file, file.Path).ContinueWith(()=> {
                            if (file.FileType.Contains("script"))
                            {
                                EdmsTasks.cweeTask streamJob = new EdmsTasks.cweeTask();
                                file.OpenReadAsync().AsTask().ContinueWith(async (System.Threading.Tasks.Task<Windows.Storage.Streams.IRandomAccessStreamWithContentType> c) => {
                                    try
                                    {
                                        using (var reader = new StreamReader(c.Result.AsStreamForRead()))
                                        {
                                            string content = await reader.ReadToEndAsync();
                                            streamJob.SetFinished(content);
                                        }
                                    }
                                    catch (Exception) { streamJob.SetFinished(""); }
                                });

                                //EdmsTasks.InsertJob(async () => {
                                //    streamJob.SetFinished(await Windows.Storage.FileIO.ReadTextAsync(file));
                                //}, true);

                                streamJob.ContinueWith(() => {
                                    JSON_Serializer_Workspace jsonObj = JsonConvert.DeserializeObject<JSON_Serializer_Workspace>(streamJob.Result);
                                    var page = AddNewTextScriptPage();
                                    page.ContinueWith(() =>
                                    {
                                        LoadWorkspace(jsonObj, page.Result);
                                    }, true);
                                }, true);
                            }
                            else
                            {
                                EdmsTasks.cweeTask streamJob = new EdmsTasks.cweeTask();
                                file.OpenReadAsync().AsTask().ContinueWith(async (System.Threading.Tasks.Task<Windows.Storage.Streams.IRandomAccessStreamWithContentType> c) => {
                                    try
                                    {
                                        using (var reader = new StreamReader(c.Result.AsStreamForRead()))
                                        {
                                            string content = await reader.ReadToEndAsync();
                                            streamJob.SetFinished(content);
                                        }
                                    }
                                    catch (Exception) { streamJob.SetFinished(""); }                                    
                                });
                                streamJob.ContinueWith(() => {
                                    JSON_Serializer_Workspace jsonObj = JsonConvert.DeserializeObject<JSON_Serializer_Workspace>(streamJob.Result);
                                    var page = AddNewVisualScriptPage();
                                    page.ContinueWith(() =>
                                    {
                                        LoadWorkspace(jsonObj, page.Result);
                                    }, true);
                                }, true);
                            }
                        }, true);
                    }
                }, true);
            }
            catch (Exception) { }
        }
        internal cweeTask<TextScriptingPage> AddNewTextScriptPage()
        {
            var scriptingPage = EdmsTasks.InsertJob(() => {
                var p = new TextScriptingPage();
                p.VM.ScriptName = "New Script";
                return p;
            }, true);
            var db = cweeXamlHelper.ThemeColor("cweeDarkBlue");
            return (EdmsTasks.cweeTask)EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() { scriptingPage, db }).ContinueWith(() => {
                var header = UWP_WaterWatchLibrary.Extensions.CreateEditableTabHeader(new Binding() { Source = scriptingPage.Result.VM, Path = new PropertyPath("ScriptName"), Mode = BindingMode.TwoWay, UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged }); // scriptingPage.VM);
                return header.ContinueWith(() => {
                    var tab = new TabViewItem()
                    {
                        Content = scriptingPage.Result,
                        IconSource = new Microsoft.UI.Xaml.Controls.SymbolIconSource() { Symbol = Windows.UI.Xaml.Controls.Symbol.Document, Foreground = db.Result },
                        Header = header.Result
                    };
                    tab.Tag = new TabViewItemViewModel(false, true);
                    tab.PointerEntered += (object sender2, PointerRoutedEventArgs e) => { tab.IsClosable = true; };
                    tab.PointerExited += (object sender2, PointerRoutedEventArgs e) => { tab.IsClosable = false; };

                    tabview.TabItems.Add(tab);
                    tabview.SelectedItem = tab;

                    EdmsTasks.cweeTask trueWhenScriptPageLoaded = new EdmsTasks.cweeTask(() => { return scriptingPage.Result; }, false, true);
                    scriptingPage.Result.Tag = trueWhenScriptPageLoaded;
                    scriptingPage.Result.Loaded += TextScriptPage_Loaded;

                    return trueWhenScriptPageLoaded; // scriptingPage.Result;
                }, true);
            }, true);
        }
        internal cweeTask<ScriptingPage> AddNewVisualScriptPage() {
            var scriptingPage = EdmsTasks.InsertJob(()=> {
                var p = new ScriptingPage();
                p.VM.ScriptName = "New Script";
                return p;
            }, true);
            var db = cweeXamlHelper.ThemeColor("cweeDarkBlue");
            return (EdmsTasks.cweeTask)EdmsTasks.cweeTask.TrueWhenCompleted(new List<EdmsTasks.cweeTask>() { scriptingPage, db }).ContinueWith(()=> {
                var header = UWP_WaterWatchLibrary.Extensions.CreateEditableTabHeader(new Binding() { Source = scriptingPage.Result.VM, Path = new PropertyPath("ScriptName"), Mode = BindingMode.TwoWay, UpdateSourceTrigger = UpdateSourceTrigger.PropertyChanged }); // scriptingPage.VM);
                return header.ContinueWith(() => {
                    var tab = new TabViewItem()
                    {
                        Content = scriptingPage.Result,
                        IconSource = new Microsoft.UI.Xaml.Controls.SymbolIconSource() { Symbol = Windows.UI.Xaml.Controls.Symbol.Document, Foreground = db.Result },
                        Header = header.Result
                    };
                    tab.Tag = new TabViewItemViewModel(true, false);
                    tab.PointerEntered += (object sender2, PointerRoutedEventArgs e) => { tab.IsClosable = true; };
                    tab.PointerExited += (object sender2, PointerRoutedEventArgs e) => { tab.IsClosable = false; };

                    tabview.TabItems.Add(tab);
                    tabview.SelectedItem = tab;

                    EdmsTasks.cweeTask trueWhenScriptPageLoaded = new EdmsTasks.cweeTask(()=> { return scriptingPage.Result; }, false, true);
                    scriptingPage.Result.Tag = trueWhenScriptPageLoaded;
                    scriptingPage.Result.Loaded += ScriptPage_Loaded;

                    return trueWhenScriptPageLoaded; // scriptingPage.Result;
                }, true);
            }, true);
        }
        private static void ScriptPage_Loaded(object sender, RoutedEventArgs e)
        {
            (sender as ScriptingPage).Loaded -= ScriptPage_Loaded;
            ((sender as ScriptingPage).Tag as EdmsTasks.cweeTask).QueueJob();
            (sender as ScriptingPage).Tag = null;
        }
        private static void TextScriptPage_Loaded(object sender, RoutedEventArgs e)
        {
            (sender as TextScriptingPage).Loaded -= TextScriptPage_Loaded;
            ((sender as TextScriptingPage).Tag as EdmsTasks.cweeTask).QueueJob();
            (sender as TextScriptingPage).Tag = null;
        }
    }

    public sealed partial class ScriptingManagerCommandBar : UserControl
    {
        public static string NewScriptNodeName() { return $"WW_{WaterWatch.RandomInt(0, 100)}_{WaterWatch.RandomInt(0, 100)}_{WaterWatch.RandomInt(0, 100)}"; }

        private ScriptingManagerCommandBarViewModel VM = new ScriptingManagerCommandBarViewModel();
        internal Storyboard storyboard;

        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged([CallerMemberName] string propertyName = null) {
            EdmsTasks.InsertJob(() => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName)), EdmsTasks.Priority.Low, true, true);
        }

        public bool ShowVisualTab { get { return VM.ShowVisualTab; } set { VM.ShowVisualTab = value; OnPropertyChanged("ShowEditTab"); OnPropertyChanged("ShowVisualTab"); } }
        public bool ShowTextTab { get { return VM.ShowTextTab; } set { VM.ShowTextTab = value; OnPropertyChanged("ShowEditTab"); OnPropertyChanged("ShowTextTab"); } }

        public TabView TabView { get{ return VM.tabview; } set{ VM.tabview = value; } }
        

        public ScriptingManagerCommandBar()
        {
            this.InitializeComponent();
        }
        ~ScriptingManagerCommandBar()
        {
            VM = null;
            storyboard = null;
        }

        
        private void CommandBar_PointerEntered(object sender, PointerRoutedEventArgs e)
        {
            if (storyboard != null) storyboard.Stop();
            storyboard = new Storyboard();
            DoubleAnimation da = new DoubleAnimation() { From = 48, To = 128, Duration = new Duration(new TimeSpan(0, 0, 0, 0, 200)), EnableDependentAnimation = true };
            Storyboard.SetTarget(da, sender as FrameworkElement);
            Storyboard.SetTargetProperty(da, "MaxHeight");
            storyboard.Children.Add(da);
            storyboard.Begin();
        }
        private void CommandBar_PointerExited(object sender, PointerRoutedEventArgs e)
        {
            if (storyboard != null) storyboard.Stop();
            storyboard = new Storyboard();
            DoubleAnimation da = new DoubleAnimation() { From = 128, To = 48, Duration = new Duration(new TimeSpan(0, 0, 0, 0, 200)), EnableDependentAnimation = true };
            Storyboard.SetTarget(da, sender as FrameworkElement);
            Storyboard.SetTargetProperty(da, "MaxHeight");
            storyboard.Children.Add(da);
            storyboard.Begin();
        }

        private void AddScriptNode_Click(object sender, RoutedEventArgs e)
        {
            ScriptingManagerCommandBarViewModel.AddScriptNode(NewScriptNodeName(), VM.GetCurrentScriptingPage());
        }

        private void BackgroundGrid_Click(object sender, RoutedEventArgs e)
        {
            var view = VM?.GetCurrentView()?.GetScriptingPage();
            if (view != null)
            {
                bool? vis = view?.VM?.workspace?.GridVisible;
                if (vis != null)
                {
                    view.VM.workspace.GridVisible = !vis.Value;
                }
            }
        }

        private void ZoomIn_Click(object sender, RoutedEventArgs e)
        {
            VM?.GetCurrentView()?.GetScriptingPage()?.VM?.workspace?.ZoomIn();
        }
        private void ZoomOut_Click(object sender, RoutedEventArgs e)
        {
            VM?.GetCurrentView()?.GetScriptingPage()?.VM?.workspace?.ZoomOut();
        }
        private void Zoom100percent_Click(object sender, RoutedEventArgs e)
        {
            VM?.GetCurrentView()?.GetScriptingPage()?.VM?.workspace?.ResetZoom();
        }

        private void SaveWorkspace_Click(object sender, RoutedEventArgs e)
        {
            // VM?.SaveWorkspace();
        }

        private void LoadWorkspace_Click(object sender, RoutedEventArgs e)
        {
            // VM?.LoadWorkspace();
        }

        private void SaveScript_Click(object sender, RoutedEventArgs e)
        {
            VM?.SaveScript();
        }

        private void SaveScriptAs_Click(object sender, RoutedEventArgs e)
        {
            VM?.SaveScriptAs();
        }

        private void SaveAll_Click(object sender, RoutedEventArgs e)
        {
            VM?.SaveAll();
        }

        private void OpenExistingScripts_Click(object sender, RoutedEventArgs e)
        {
            VM?.OpenExistingScripts();
        }

        private void WRITE_TIMES_CLICK(object sender, RoutedEventArgs e)
        {
            EdmsTasks.DoWriteAllJobTimes = true;
        }

        private void CopyCode_Click(object sender, RoutedEventArgs e)
        {

        }

        private void SaveCodeAs_Click(object sender, RoutedEventArgs e)
        {

        }

        private void Email_Click(object sender, RoutedEventArgs e)
        {

        }

        private void ReportProblem_Click(object sender, RoutedEventArgs e)
        {

        }

        private void FeatureSuggest_Click(object sender, RoutedEventArgs e)
        {

        }

        private void NewScript_Click(object sender, RoutedEventArgs e)
        {
            VM?.AddNewTextScriptPage();
        }

        private void NewVisualScript_Click(object sender, RoutedEventArgs e)
        {
            VM?.AddNewVisualScriptPage();
        }
    }
}
