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

#pragma once
#include "Precompiled.h"
#include "chaiscript_wrapper.h"
#include "cweeAny.h"
#include "SharedPtr.h"
#include "cweeJob.h"
#include "cwee_math.h"
#include "Strings.h"
#include "List.h"
#include "cweeScheduler.h"

// Async Support
namespace chaiscript {
    class FutureObjContent {
    public:
        FutureObjContent() : bv(false, chaiscript::Boxed_Value()), prom(), fut() {};
        FutureObjContent(cweeAny promise_p, std::future<chaiscript::Boxed_Value>&& fut_p) : bv(false, chaiscript::Boxed_Value()), prom(promise_p), fut(std::forward<std::future<chaiscript::Boxed_Value>>(fut_p)) {};

        cweeUnion<bool, cweeAny/*chaiscript::Boxed_Value*/> bv; // after the user asks for it, it is stored here to prevent the double-tap throw from std::future
        std::future<chaiscript::Boxed_Value> fut; // grants the ability to 'check' for completion.
        cweeAny prom; // contains the underlying result until the user asks for it.

        AUTO GetResult() {
            if (!bv.get<0>()) {
                bv.get<1>() = make_cwee_shared<chaiscript::Boxed_Value>(std::move(fut.get()));
                bv.get<0>() = true;
                // c.bv = cweeUnion<bool, cweeAny/*chaiscript::Boxed_Value*/>(true, cweeAny(make_cwee_shared<chaiscript::Boxed_Value>(std::move(fut.get())))/*std::move(fut.get())*/);
            }
            return *bv.get<1>().cast<chaiscript::Boxed_Value*>();
        };

    };

    class FutureObj {
    public:
        FutureObj() : 
            content(make_cwee_shared<FutureObjContent>(new FutureObjContent()))
            , awaiter() {};
        FutureObj(cweeAny promise_p, std::future<chaiscript::Boxed_Value>&& fut_p, cweeJob const& awaiter_p) : 
            content(make_cwee_shared<FutureObjContent>(promise_p, std::forward<std::future<chaiscript::Boxed_Value>>(fut_p)))
            , awaiter(awaiter_p) {};

        bool valid() const {
            using namespace std::chrono_literals;
            return (content.cast<FutureObjContent&>().fut.wait_for(0s) == std::future_status::ready);
        };
        AUTO wait() {
            AUTO c = content.cast<FutureObjContent&>();
            if (!c.bv.get<0>()) {
                while (!valid()) {
                    awaiter.AwaitAll(); // tries to evaluate everything -- note that this can evaluate other jobs ahead of this one if needed / timing doesn't work out.
                }
                c.bv.get<1>() = make_cwee_shared<chaiscript::Boxed_Value>(std::move(c.fut.get()));
                c.bv.get<0>() = true;

                // c.bv = cweeUnion<bool, cweeAny/*chaiscript::Boxed_Value*/>(true, cweeAny(make_cwee_shared<chaiscript::Boxed_Value>(std::move(fut.get())))/*std::move(fut.get())*/);
            }
            return *c.bv.get<1>().cast<chaiscript::Boxed_Value*>();
        };
        AUTO get() { return wait(); };

        AUTO continue_with(std::function<chaiscript::Boxed_Value()> const& t_func) {
            cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());

            auto awaiter2 = awaiter.ContinueWith(cweeJob([](std::function<chaiscript::Boxed_Value()>& func, std::promise<chaiscript::Boxed_Value>& promise) {
                promise.set_value(func());
                }, t_func, p));

            return FutureObj(
                p, /* promise container */
                p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                awaiter2 /* job task */
            );
        };
        AUTO continue_with(std::function<void()> const& t_func) {
            cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());

            auto awaiter2 = awaiter.ContinueWith(cweeJob([](std::function<void()>& func, std::promise<chaiscript::Boxed_Value>& promise) {
                func();
                promise.set_value(chaiscript::Boxed_Value());
            }, t_func, p));

            return FutureObj(
                p, /* promise container */
                p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                awaiter2 /* job task */
            );

            /*awaiter.ContinueWith(cweeJob([](std::function<chaiscript::Boxed_Value()>& func) {
                func();
                }, t_func));*/
        };
        
        AUTO continue_with(std::function<chaiscript::Boxed_Value(chaiscript::Boxed_Value)> const& t_func) {
            cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());

            auto awaiter2 = awaiter.ContinueWith(cweeJob([](std::function<chaiscript::Boxed_Value(chaiscript::Boxed_Value)>& func, std::promise<chaiscript::Boxed_Value>& promise, FutureObjContent& prevResult) {
                // handle the prev result
                promise.set_value(func(prevResult.GetResult()));
            }, t_func, p, content));

            return FutureObj(
                p, /* promise container */
                p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                awaiter2 /* job task */
            );
        };
        AUTO continue_with(std::function<void(chaiscript::Boxed_Value)> const& t_func) {
            cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());

            auto awaiter2 = awaiter.ContinueWith(cweeJob([](std::function<void(chaiscript::Boxed_Value)>& func, std::promise<chaiscript::Boxed_Value>& promise, FutureObjContent& prevResult) {
                // handle the prev result
                func(prevResult.GetResult());
                promise.set_value(chaiscript::Boxed_Value());
            }, t_func, p, content));

            return FutureObj(
                p, /* promise container */
                p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                awaiter2 /* job task */
            );
        };

    protected:
        cweeAny content;
        cweeJob awaiter;
    };
};

// UI Support and interface
namespace chaiscript {
#define AddMemberToScriptFromClass(Name) scriptingLanguage.add(chaiscript::fun(static_cast<decltype(Name)(ThisType::*)>(&ThisType::Name)), #Name)
#define AddFuncToScriptFromClass(Name) scriptingLanguage.add(chaiscript::fun(&ThisType::Name), #Name)
#define AddBasicClassTemplate() { \
                        scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName()); \
                        scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName()); \
                        scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName()); \
                        scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "="); \
                    }

    class UI_Color {
    public:
        using ThisType = typename UI_Color;
        static  std::string	ThisTypeName() { return "UI_Color"; };

        UI_Color() : R(128), G(128), B(128), A(255) {};
        UI_Color(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {};
        UI_Color(UI_Color const& r) : R(r.R), G(r.G), B(r.B), A(r.A) {};

        float R, G, B, A;
        UI_Color Lerp(const UI_Color& whenOne, float lerp) const {
            UI_Color out;

            out.R = whenOne.R * lerp + this->R * (1.0f - lerp);
            out.G = whenOne.G * lerp + this->G * (1.0f - lerp);
            out.B = whenOne.B * lerp + this->B * (1.0f - lerp);
            out.A = whenOne.A * lerp + this->A * (1.0f - lerp);

            return out;
        };

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::constructor<ThisType(float, float, float, float)>(), ThisTypeName());
            scriptingLanguage.add(chaiscript::fun([](UI_Color& a) { return std::string(cweeStr::printf("[%s, %s, %s, %s]", cweeStr(a.R).c_str(), cweeStr(a.G).c_str(), cweeStr(a.B).c_str(), cweeStr(a.A).c_str()).c_str()); }), "to_string");

            AddMemberToScriptFromClass(R);
            AddMemberToScriptFromClass(G);
            AddMemberToScriptFromClass(B);
            AddMemberToScriptFromClass(A);
            AddFuncToScriptFromClass(Lerp);

            scriptingLanguage.eval(R"(
			    def `=`(UI_Color a, string  b){
				    return fun[a,b](){
					    a = App.ThemeColor(b);
					    return a;
				    }();				
			    }			
		    )");
        };
    };
    class UI_App {
    public:
        using ThisType = typename UI_App;
        static std::string	ThisTypeName() { return "UI_App"; };

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            scriptingLanguage.eval(R"(
			class AppImpl{};
			attr AppImpl::Think;
			attr AppImpl::SelectFolder;
			attr AppImpl::SavePassword;
			attr AppImpl::LoadPassword;
			attr AppImpl::ThemeColor;
			attr AppImpl::GetUserName;
			attr AppImpl::GetMousePosition;
			attr AppImpl::SetClipboard;
			attr AppImpl::GetClipboard;
			attr AppImpl::SaveSetting;
			attr AppImpl::GetSetting;

			def AppImpl::AppImpl(){ 
				this.Think = fun[](){}; 
				this.SelectFolder = fun[](){
					return AppFunction("OS_SelectFolder");
				};
				this.SavePassword = fun[](string server, string username, string password){
					AppFunction("OS_SavePassword", server, username, password);
				};
				this.LoadPassword = fun[](string server, string username){
					return AppFunction("OS_LoadPassword", server, username);
				};	
				this.ThemeColor = fun[](string key){
					var& col = eval(AppFunction("OS_ThemeColor", key));
					var& out = UI_Color();
					out.R = col[0]; out.G = col[1]; out.B = col[2]; out.A = col[3];
					return out;
				};
				this.GetUserName = fun[](){
					return AppFunction("OS_GetUserName");
				};
				this.GetMousePosition = fun[](){
					return eval(AppFunction("OS_GetMousePosition"));
				};				
				this.SetClipboard = fun[](string content){
					AppFunction("OS_SetClipboard", content);
				};
				this.GetClipboard = fun[](){
					return AppFunction("OS_GetClipboard");
				};
				this.SaveSetting = fun[](string key, string value){
					AppFunction("OS_SaveSetting", key, value);
				};
				this.GetSetting = fun[](string key){
					return AppFunction("OS_GetSetting", key);
				};
				this.set_explicit(true);
			};
			global App = AppImpl();

			{
				App.set_explicit(false);
				var& colMap = Map();
				App.ThemeColor = fun[colMap](string key){
					if (!colMap .contains(key)){
						var& col = eval(AppFunction("OS_ThemeColor", key));
						var& out = UI_Color();
						out.R = col[0]; out.G = col[1]; out.B = col[2]; out.A = col[3];		
						colMap[key] := out;
					}
					return colMap[key] ;
				};
				App.set_explicit(true);
			}

            def AppImpl::SelectFile(){
	            return AppFunction("OS_SelectFile");
            };
            def AppImpl::SelectFile(Function AndThen){
	            var out = this.SelectFile();
	            try{ 
                    AndThen();
                }catch(e){
                    try{ 
                        AndThen(out);
                    }
                }
	            return out;
            };


		)");
        };
    };

#define AddFrameworkElementCharacteristicsToScriptFromClass() {\
    AddMemberToScriptFromClass(Version); \
    AddMemberToScriptFromClass(UniqueName); \
    scriptingLanguage.add(chaiscript::fun(&ThisType::Update), "Update"); \
    scriptingLanguage.add(chaiscript::fun(&ThisType::AddTask), "AddTask"); \
    scriptingLanguage.add(chaiscript::fun(&ThisType::GetTasks), "GetTasks"); \
    AddMemberToScriptFromClass(Tasks); \
    AddMemberToScriptFromClass(Opacity); \
    AddMemberToScriptFromClass(Width); \
    AddMemberToScriptFromClass(Height); \
    AddMemberToScriptFromClass(VerticalAlignment); \
    AddMemberToScriptFromClass(HorizontalAlignment); \
    AddMemberToScriptFromClass(Tag); \
    AddMemberToScriptFromClass(Name); \
    AddMemberToScriptFromClass(MinWidth); \
    AddMemberToScriptFromClass(MinHeight); \
    AddMemberToScriptFromClass(MaxWidth); \
    AddMemberToScriptFromClass(MaxHeight); \
    AddMemberToScriptFromClass(Margin); \
    AddMemberToScriptFromClass(OnLoaded); \
}
    class UI_FrameworkElement {
    public:
        using ThisType = typename UI_FrameworkElement;
        static std::string	ThisTypeName() { return "UI_FrameworkElement"; };

        virtual ~UI_FrameworkElement() {};

        std::vector<chaiscript::Boxed_Value> Tasks;
        int				UniqueName = cweeStr::hash(cweeStr::printf("%i_%i", cweeRandomInt(0, 100000), cweeRandomInt(0, 100000)));
        int				Version = 0;
        int				Update() { return AddTask("Update"); };
        int				AddTask(cweeStr const& task) {
            Tasks.push_back(chaiscript::Boxed_Value(std::string(task.c_str())));
            return ++Version;
        };
        std::vector<chaiscript::Boxed_Value> GetTasks() {
            std::vector<chaiscript::Boxed_Value> out = Tasks;
            Tasks = std::vector<chaiscript::Boxed_Value>();
            return out;
        };

        double			Opacity = -1;
        double 			Width = -1;
        double 			Height = -1;
        cweeStr 		VerticalAlignment = "Stretch";
        cweeStr			HorizontalAlignment = "Stretch";
        chaiscript::Boxed_Value Tag;
        cweeStr			Name;
        double 			MinWidth = -1;
        double 			MinHeight = -1;
        double 			MaxWidth = -1;
        double 			MaxHeight = -1;
        cweeStr 		Margin = "0,0,0,0";
        chaiscript::Boxed_Value OnLoaded;


        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            AddFrameworkElementCharacteristicsToScriptFromClass();
        };
    };
    class UI_Rectangle final : public UI_FrameworkElement {
    public:
        using ThisType = typename UI_Rectangle;
        static  std::string	ThisTypeName() { return "UI_Rectangle"; };

        virtual ~UI_Rectangle() {};

        UI_Color 		Fill;

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
#if 0
            AddFrameworkElementCharacteristicsToScriptFromClass();
#else
            //AddMemberToScriptFromClass(Version); 
            //AddMemberToScriptFromClass(UniqueName); 
            //scriptingLanguage.add(chaiscript::fun(&ThisType::Update), "Update"); 
            //scriptingLanguage.add(chaiscript::fun(&ThisType::AddTask), "AddTask"); 
            //scriptingLanguage.add(chaiscript::fun(&ThisType::GetTasks), "GetTasks"); 
            //AddMemberToScriptFromClass(Tasks); 
            //AddMemberToScriptFromClass(Opacity); 
            //AddMemberToScriptFromClass(Width); 
            //AddMemberToScriptFromClass(Height); 
            //AddMemberToScriptFromClass(VerticalAlignment); 
            //AddMemberToScriptFromClass(HorizontalAlignment); 
            //AddMemberToScriptFromClass(Tag); 
            // AddMemberToScriptFromClass(Name); 
            //AddMemberToScriptFromClass(MinWidth); 
            //AddMemberToScriptFromClass(MinHeight); 
            //AddMemberToScriptFromClass(MaxWidth); 
            //AddMemberToScriptFromClass(MaxHeight); 
            //AddMemberToScriptFromClass(Margin);
#endif
            AddMemberToScriptFromClass(Fill);
        };
    };
    class UI_TextBlock final: public UI_FrameworkElement {
    public:
        using ThisType = typename UI_TextBlock;
        static  std::string	ThisTypeName() { return "UI_TextBlock"; };

        UI_TextBlock() {};
        UI_TextBlock(const cweeStr& text) {
            Text = text;
        };
        virtual ~UI_TextBlock() {};

        cweeStr 		TextWrapping = "NoWrap";
        cweeStr 		TextTrimming = "None";
        cweeStr 		TextAlignment = "Center";
        cweeStr			Text;
        cweeStr 		Padding = "0,0,0,0";
        UI_Color 		Foreground;
        double 			FontSize = 18;
        cweeStr 		HorizontalTextAlignment = "Center";

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::constructor<ThisType(const cweeStr&)>(), ThisTypeName());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(TextWrapping);
            AddMemberToScriptFromClass(TextTrimming);
            AddMemberToScriptFromClass(TextAlignment);
            AddMemberToScriptFromClass(Text);
            AddMemberToScriptFromClass(Padding);
            AddMemberToScriptFromClass(Foreground);
            AddMemberToScriptFromClass(FontSize);
            AddMemberToScriptFromClass(HorizontalTextAlignment);
        };
    };
    class UI_Image final : public UI_FrameworkElement {
    public:
        using ThisType = typename UI_Image;
        static  std::string	ThisTypeName() { return "UI_Image"; };

        UI_Image() {};
        UI_Image(const cweeStr& imagePath) {
            ImagePath = imagePath;
        };
        virtual ~UI_Image() {};

        cweeStr 		ImagePath;
        cweeStr			Stretch = "UniformToFill";

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::constructor<ThisType(const cweeStr&)>(), ThisTypeName());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(ImagePath);
            AddMemberToScriptFromClass(Stretch);
        };
    };
    class UI_WebView final : public UI_FrameworkElement {
    public:
        using ThisType = typename UI_WebView;
        static  std::string	ThisTypeName() { return "UI_WebView"; };

        UI_WebView() {};
        UI_WebView(const cweeStr& source) {
            Source = source;
        };
        virtual ~UI_WebView() {};

        cweeStr 															Source;
        std::map<std::string, chaiscript::Boxed_Value>						ResultQueue;
        int																	NumResultsQueued = 0;
        chaiscript::Boxed_Value												NavigationCompleted;

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::constructor<ThisType(const cweeStr&)>(), ThisTypeName());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            // AddFrameworkElementCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(Source);
            AddMemberToScriptFromClass(NavigationCompleted);

            AddMemberToScriptFromClass(ResultQueue);
            AddMemberToScriptFromClass(NumResultsQueued);
            scriptingLanguage.eval(R"(
			    def UI_WebView::Navigate(string where) {
				    this.AddTask("Navigate ${where}");
			    };
			    def UI_WebView::InvokeScript(string what) {
				    this.AddTask("InvokeScript -1 ${what}");
			    };
			    def UI_WebView::InvokeScript(string what, Function DoWithOrAfterResult) {
				    var j = ++this.NumResultsQueued;
				    this.ResultQueue["${j}"] := DoWithOrAfterResult;
				    this.AddTask("InvokeScript ${j} ${what}");
			    };
		    )");
        };
    };

#define AddPanelCharacteristicsToScriptFromClass(){\
    AddMemberToScriptFromClass(Background); \
    AddMemberToScriptFromClass(Children); \
    AddMemberToScriptFromClass(Padding); \
    };
    class UI_Panel : public UI_FrameworkElement {
    public:
        using ThisType = typename  UI_Panel;
        static std::string	ThisTypeName() { return "UI_Panel"; };

        virtual ~UI_Panel() {};

        UI_Color 		Background = UI_Color(0.0f, 0.0f, 0.0f, 0.0f);
        cweeThreadedList<chaiscript::Boxed_Value> Children;
        cweeStr 		Padding = "0,0,0,0";

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();

            AddPanelCharacteristicsToScriptFromClass();
        };
    };
    class UI_Grid final : public UI_Panel {
    public:
        using ThisType = typename  UI_Grid;
        static  std::string	ThisTypeName() { return "UI_Grid"; };

        virtual ~UI_Grid() {};

        std::vector<std::string> ColumnDefinitions; // "1*", "5", "Auto"
        std::vector<std::string> RowDefinitions; // "1*", "5", "Auto"
        std::vector<std::string> ChildPositions; // "0,0" or "1,1", etc. 
        double			ColumnSpacing = 0;
        double			RowSpacing = 0;

        void 			AddChild(chaiscript::Boxed_Value const& child, int row, int column, int rowSpan, int columnSpan) {
            this->Children.push_back(child);
            this->ChildPositions.push_back(cweeStr::printf("%i,%i,%i,%i", row, column, rowSpan, columnSpan).c_str());
        };

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();

            scriptingLanguage.add(chaiscript::base_class<UI_Panel, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddPanelCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(ColumnDefinitions);
            AddMemberToScriptFromClass(RowDefinitions);
            AddMemberToScriptFromClass(ChildPositions);
            AddMemberToScriptFromClass(ColumnSpacing);
            AddMemberToScriptFromClass(RowSpacing);

            scriptingLanguage.add(chaiscript::fun(&ThisType::AddChild), "AddChild");
        };
    };
    class UI_StackPanel final : public UI_Panel {
    public:
        using ThisType = typename  UI_StackPanel;
        static  std::string	ThisTypeName() { return "UI_StackPanel"; };

        virtual ~UI_StackPanel() {};

        cweeStr 		Orientation = "Vertical";
        double			Spacing = 0;

        void 			AddChild(chaiscript::Boxed_Value const& child) {
            this->Children.Append(child);
        };

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_Panel, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddPanelCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(Orientation);
            AddMemberToScriptFromClass(Spacing);

            scriptingLanguage.add(chaiscript::fun(&ThisType::AddChild), "AddChild");
        };
    };

#define AddControlCharacteristicsToScriptFromClass(){\
    AddMemberToScriptFromClass(IsEnabled); \
    AddMemberToScriptFromClass(Padding); \
    };

    class UI_Control : public UI_FrameworkElement {
    public:
        using ThisType = typename  UI_Control;
        static std::string	ThisTypeName() { return "UI_Control"; };

        virtual ~UI_Control() {};

        cweeStr 		Padding = "0,0,0,0";
        bool			IsEnabled = true;

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            AddControlCharacteristicsToScriptFromClass();
        };
    };
    class UI_Button final : public UI_Control {
    public:
        using ThisType = typename  UI_Button;
        static  std::string	ThisTypeName() { return "UI_Button"; };

        virtual ~UI_Button() {};

        chaiscript::Boxed_Value		Content;
        chaiscript::Boxed_Value		Clicked; // Meant to be a lamda function, called when checkbox is unchecked

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddControlCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(Content);
            AddMemberToScriptFromClass(Clicked);
        };
    };
    class UI_CheckBox final : public UI_Control {
    public:
        using ThisType = typename   UI_CheckBox;
        static  std::string	ThisTypeName() { return "UI_CheckBox"; };

        virtual ~UI_CheckBox() {};

        bool						IsChecked = true;
        chaiscript::Boxed_Value		Checked; // Meant to be a lamda function, called when checkbox is checked
        chaiscript::Boxed_Value		Unchecked; // Meant to be a lamda function, called when checkbox is unchecked

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddControlCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(IsChecked);
            AddMemberToScriptFromClass(Checked);
            AddMemberToScriptFromClass(Unchecked);
        };
    };
    class UI_Slider final : public UI_Control {
    public:
        using ThisType = typename   UI_Slider;
        static  std::string	ThisTypeName() { return "UI_Slider"; };

        virtual ~UI_Slider() {};

        double						Value = 0;
        double						SmallChange = 1;
        double						LargeChange = 10;
        double						Minimum = 0;
        double						Maximum = 100;
        double						StepFrequency = 1;
        double						TickFrequency = 1;
        chaiscript::Boxed_Value		ValueChanged; // Meant to be a lamda function, called when slider value changes

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddControlCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(Value);
            AddMemberToScriptFromClass(SmallChange);
            AddMemberToScriptFromClass(LargeChange);
            AddMemberToScriptFromClass(Minimum);
            AddMemberToScriptFromClass(Maximum);
            AddMemberToScriptFromClass(StepFrequency);
            AddMemberToScriptFromClass(TickFrequency);
            AddMemberToScriptFromClass(ValueChanged);
        };
    };
    class UI_ToggleSwitch final : public UI_Control {
    public:
        using ThisType = typename   UI_ToggleSwitch;
        static  std::string	ThisTypeName() { return "UI_ToggleSwitch"; };

        virtual ~UI_ToggleSwitch() {};

        bool						IsOn = true;
        chaiscript::Boxed_Value		OnContent; // Meant to be something the UI can draw
        chaiscript::Boxed_Value		OffContent; // Meant to be something the UI can draw
        chaiscript::Boxed_Value		Toggled; // Meant to be a lamda function, called when toggleswitch is toggled

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddControlCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(IsOn);
            AddMemberToScriptFromClass(OnContent);
            AddMemberToScriptFromClass(OffContent);
            AddMemberToScriptFromClass(Toggled);
        };
    };
    class UI_ListView final : public UI_Control {
    public:
        using ThisType = typename   UI_ListView;
        static std::string	ThisTypeName() { return "UI_ListView"; };

        UI_ListView() {

        };
        UI_ListView(const std::vector<chaiscript::Boxed_Value>& inputItems) {
            Items = inputItems;
        };
        virtual ~UI_ListView() {};

        UI_Color 					Background = UI_Color(0.0f, 0.0f, 0.0f, 0.0f);
        bool						CanReorderItems = false;
        chaiscript::Boxed_Value		Header;
        chaiscript::Boxed_Value		Footer;
        cweeThreadedList<chaiscript::Boxed_Value> Items;

        void 			AddItem(chaiscript::Boxed_Value const& item) {
            this->Items.push_back(item);
        };

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::constructor<ThisType(const std::vector<chaiscript::Boxed_Value>&)>(), ThisTypeName());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddControlCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(Background);
            AddMemberToScriptFromClass(CanReorderItems);
            AddMemberToScriptFromClass(Header);
            AddMemberToScriptFromClass(Footer);
            AddMemberToScriptFromClass(Items);

            scriptingLanguage.add(chaiscript::fun(&ThisType::AddItem), "AddItem");
        };
    };
    class UI_TabView final : public UI_Control {
    public:
        using ThisType = typename   UI_TabView;
        static std::string	ThisTypeName() { return "UI_TabView"; };

        virtual ~UI_TabView() {};

        UI_Color 					Background = UI_Color(0.0f, 0.0f, 0.0f, 0.0f);
        bool						CanReorderTabs = false;
        bool						CanDragTabs = false;
        cweeThreadedList<chaiscript::Boxed_Value> TabItems;
        cweeThreadedList<chaiscript::Boxed_Value> HeaderItems;

        void 			AddTab(chaiscript::Boxed_Value const& tab, chaiscript::Boxed_Value const& header) {
            this->TabItems.push_back(tab);
            this->HeaderItems.push_back(header);
        };

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddControlCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(Background);
            AddMemberToScriptFromClass(CanReorderTabs);
            AddMemberToScriptFromClass(CanDragTabs);
            AddMemberToScriptFromClass(TabItems);
            AddMemberToScriptFromClass(HeaderItems);

            scriptingLanguage.add(chaiscript::fun(&ThisType::AddTab), "AddTab");
        };
    };
    class UI_Expander final : public UI_Control {
    public:
        using ThisType = typename   UI_Expander;
        static  std::string	ThisTypeName() { return "UI_Expander"; };

        virtual ~UI_Expander() {};

        bool						IsExpanded = true;
        cweeStr						ExpandDirection = "Down";

        chaiscript::Boxed_Value		Header; // Meant to be something the UI can draw
        chaiscript::Boxed_Value		Content; // Meant to be something the UI can draw
        chaiscript::Boxed_Value		Expanding; // Meant to be a lamda function, called when Expander is expanding

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddControlCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(IsExpanded);
            AddMemberToScriptFromClass(ExpandDirection);
            AddMemberToScriptFromClass(Header);
            AddMemberToScriptFromClass(Content);
            AddMemberToScriptFromClass(Expanding);
        };
    };
    class UI_TextBox final : public UI_Control {
    public:
        using ThisType = typename UI_TextBox;
        static  std::string	ThisTypeName() { return "UI_TextBox"; };

        cweeStr						Text;
        cweeStr						PlaceholderText;
        chaiscript::Boxed_Value		TextChanged; // Meant to be a lamda function, called when checkbox is unchecked

        UI_TextBox() {};
        UI_TextBox(const cweeStr& text) {
            Text = text;
        };
        virtual ~UI_TextBox() {};

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::constructor<ThisType(const cweeStr&)>(), ThisTypeName());
            scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddControlCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(Text);
            AddMemberToScriptFromClass(PlaceholderText);
            AddMemberToScriptFromClass(TextChanged);
        };
    };
    class UI_Plot final : public UI_Control {
    public:
        using ThisType = typename UI_Plot;
        static  std::string	ThisTypeName() { return "UI_Plot"; };

        virtual ~UI_Plot() {};

        UI_Color 									Background = UI_Color(0.0f, 0.0f, 0.0f, 0.0f);
        UI_Color 									AxisColor = UI_Color(0.0f, 0.0f, 0.0f, 0.0f);
        UI_Color 									FontColor = UI_Color(0.0f, 0.0f, 0.0f, 0.0f);
        bool 										HorizontalValuesAreDates = true;
        cweeStr										VerticalAxisTitle;
        cweeStr										HorizontalAxisTitle;
        cweeThreadedList<chaiscript::Boxed_Value>	PlotItems;
        cweeThreadedList<chaiscript::Boxed_Value>	PlotColors;
        cweeThreadedList<chaiscript::Boxed_Value>	PlotModes;
        double										HorizontalAxisMax = 0;
        double										HorizontalAxisMin = 0;
        double										VerticalAxisMin = 0;
        double										VerticalAxisMax = 0;

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

            //AddFrameworkElementCharacteristicsToScriptFromClass();
            //AddControlCharacteristicsToScriptFromClass();

            AddMemberToScriptFromClass(Background);
            AddMemberToScriptFromClass(AxisColor);
            AddMemberToScriptFromClass(FontColor);
            AddMemberToScriptFromClass(HorizontalValuesAreDates);
            AddMemberToScriptFromClass(VerticalAxisTitle);
            AddMemberToScriptFromClass(HorizontalAxisTitle);

            AddMemberToScriptFromClass(HorizontalAxisMax);
            AddMemberToScriptFromClass(HorizontalAxisMin);
            AddMemberToScriptFromClass(VerticalAxisMin);
            AddMemberToScriptFromClass(VerticalAxisMax);

            AddMemberToScriptFromClass(PlotItems);
            AddMemberToScriptFromClass(PlotColors);
            AddMemberToScriptFromClass(PlotModes);

            scriptingLanguage.eval(R"(
			    def UI_Plot::AddPlot(values, UI_Color col, string mode) : (values.type_name() == "Pattern" || values.type_name() == "BalancedPattern") {
				    this.PlotItems.push_back_ref(values);
				    this.PlotColors.push_back_ref(col);
				    this.PlotModes.push_back_ref(mode);
				    return;
			    };
		    )");
        };
    };

    class UI_MapElement : public UI_FrameworkElement {
    public:
        using ThisType = typename UI_MapElement;
        static  std::string	ThisTypeName() { return "UI_MapElement"; };
        UI_MapElement() {};
        virtual ~UI_MapElement() {};

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
        };
    };
    class UI_MapIcon final : public UI_MapElement {
    public: 
        using ThisType = typename UI_MapIcon;
        static  std::string	ThisTypeName() { return "UI_MapIcon"; };

        UI_MapIcon() {
            IconPathGeometry = "M 0 0 ZU 0 0 100 100 50 50";
        };
        UI_MapIcon(double Long, double Lat) {
            longitude = Long;
            latitude = Lat;
            IconPathGeometry = "M 0 0 ZU 0 0 100 100 50 50";
        };
        virtual ~UI_MapIcon() {};

        UI_Color    color;
        double      size = 12;
        double      longitude = 0;
        double      latitude = 0;
        bool        HideOnCollision = true;
        cweeStr     IconPathGeometry = "M 0 0 ZU 0 0 100 100 50 50";
        cweeStr     Label = "";

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_MapElement, ThisType>());
            scriptingLanguage.add(chaiscript::constructor<ThisType(double,double)>(), ThisTypeName());

            AddMemberToScriptFromClass(color);
            AddMemberToScriptFromClass(size);
            AddMemberToScriptFromClass(longitude);
            AddMemberToScriptFromClass(latitude);
            AddMemberToScriptFromClass(HideOnCollision);
            AddMemberToScriptFromClass(IconPathGeometry);
            AddMemberToScriptFromClass(Label);
        };
    };
    class UI_MapPolyline final : public UI_MapElement {
    public:
        using ThisType = typename UI_MapPolyline;
        static std::string	ThisTypeName() { return "UI_MapPolyline"; };

        UI_MapPolyline() : coordinates(cweeThreadedList<std::pair<double, double>>()), color(UI_Color()), dashed(false), thickness(2) {};
        virtual ~UI_MapPolyline() {};
        void AddPoint(double X, double Y) {
            coordinates.Append(std::pair<double, double>(X, Y));
        };

        cweeThreadedList<std::pair<double, double>> coordinates;
        UI_Color	color;
        bool		dashed;
        int			thickness;

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_MapElement, ThisType>());

            AddMemberToScriptFromClass(color);
            AddMemberToScriptFromClass(coordinates);
            AddMemberToScriptFromClass(dashed);
            AddMemberToScriptFromClass(thickness);

            AddFuncToScriptFromClass(AddPoint);
        };
    };
#if 0
    class UI_MapBackground final : public UI_MapElement {
    public:
        using ThisType = typename UI_MapBackground;
        static std::string	ThisTypeName() { return "UI_MapBackground"; };

        UI_MapBackground() : data(cweeInterpolatedMatrix<float>()), minColor(UI_Color()), maxColor(UI_Color()) {};
        virtual ~UI_MapBackground() {};
        UI_Color	ColorForPosition(double X, double Y) {
            float delta = (data.GetCurrentValue(X, Y) - data.GetMinValue()) / data.GetMaxValue(); // 0 to 1		
            return minColor.Lerp(maxColor, delta);
        };

        cweeInterpolatedMatrix<float> data;
        UI_Color minColor;
        UI_Color maxColor;

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_MapElement, ThisType>());

            AddMemberToScriptFromClass(data);
            AddMemberToScriptFromClass(minColor);
            AddMemberToScriptFromClass(maxColor);

            AddFuncToScriptFromClass(ColorForPosition);
        };
    };
#endif
    class UI_MapLayer : public UI_MapElement {
    public:
        using ThisType = typename UI_MapLayer;
        static  std::string	ThisTypeName() { return "UI_MapLayer"; };
        virtual ~UI_MapLayer() {};

        std::vector<chaiscript::Boxed_Value> Children;
        void SetVisibility(bool visible) {
            AddTask(cweeStr("SetVisibility ") + cweeStr(visible ? "1" : "0"));
        }

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_MapElement, ThisType>());

            AddMemberToScriptFromClass(Children);
            AddFuncToScriptFromClass(SetVisibility);

            scriptingLanguage.eval(R"(
			    def UI_MapLayer::AddChild(child) : child.is_type("UI_MapElement") {
				    this.Children.push_back_ref(child);
			    };
		    )");
        };
    };
    class UI_Map : public UI_MapElement {
    public:
        using ThisType = typename UI_Map;
        static  std::string	ThisTypeName() { return "UI_Map"; };
        virtual ~UI_Map() {};

        std::vector<chaiscript::Boxed_Value> Layers;

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            AddBasicClassTemplate();
            scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
            scriptingLanguage.add(chaiscript::base_class<UI_MapElement, ThisType>());

            AddMemberToScriptFromClass(Layers);
            scriptingLanguage.eval(R"(
			    def UI_Map::AddLayer(child) : child.is_type("UI_MapLayer") {
				    this.Layers.push_back_ref(child);
			    };
		    )");
        };
    };

#undef AddControlCharacteristicsToScriptFromClass
#undef AddPanelCharacteristicsToScriptFromClass
#undef AddFrameworkElementCharacteristicsToScriptFromClass
#undef AddBasicClassTemplate
#undef AddMemberToScriptFromClass
#undef AddFuncToScriptFromClass
};

// cwee scheduler support (optimizations)
namespace chaiscript {
#define OPT_FUNC_DIM(NUMDIM, NUMPOLICIES, _min_) case (NUMDIM) : { \
        ParticleSwarm_OptimizationManagementTool<_min_> ramt(NUMDIM, NUMPOLICIES); \
		ramt.lower_constraints() = lowerBound; \
		ramt.upper_constraints() = upperBound; \
		toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations); \
		break; \
	}
    class ChaiscriptOptimizationObj : public cweeOptimizer::sharedClass {
    public:
        int numIterationsFailedImprovement = 0;
    };
//#define ReturnFutureObjFromOptimizations
    class ChaiscriptOptimizations {
    public:
        static 
#ifdef ReturnFutureObjFromOptimizations
            FutureObj 
#else
            chaiscript::small_vector<chaiscript::Boxed_Value>
#endif
            OptimizeFunction(
                bool minimize,
                std::function<chaiscript::Boxed_Value(chaiscript::small_vector<chaiscript::Boxed_Value>)> const& per_sample_Function, // double(Vector<double>)
                chaiscript::small_vector<chaiscript::Boxed_Value> const& lowBound_Vector,
                chaiscript::small_vector<chaiscript::Boxed_Value> const& highBound_Vector,
                cweeStr optimizationType = "Genetic"
            )
        {
            optimizationType = optimizationType.BestMatch({ cweeStr("Random"), cweeStr("Genetic"), cweeStr("PSO") });

            cweeThreadedList<float> lowerBound, upperBound;
            for (auto& bv : lowBound_Vector) lowerBound.push_back(boxed_cast<double>(bv));
            for (auto& bv : highBound_Vector) upperBound.push_back(boxed_cast<double>(bv));
            std::function todo = [=](cweeThreadedList<float> const& x)-> float {
                chaiscript::small_vector< chaiscript::Boxed_Value > bv;
                for (auto& v : x) { bv.push_back(chaiscript::var((double)v)); }
                AUTO sampleF = per_sample_Function(bv);
                AUTO result = boxed_cast<double>(sampleF);
                return result;
            };

            int numDimensions = cweeMath::min(lowerBound.Num(), upperBound.Num());
            constexpr int maxIterations = 1000; // 1000 iterations
            constexpr float eps = 0.000001f;

            if (numDimensions <= 0) {
                throw std::exception("Not enough dimensions for optimization.");
                // return chaiscript::small_vector< chaiscript::Boxed_Value >();
            }

            using sharedObjType = ChaiscriptOptimizationObj; // cweeOptimizer::sharedClass;

            std::function objFunc = [=](cweeThreadedList<u64>& policy) -> u64 {
                return todo(policy);
            };
            std::function isFinishedFunc = [=](sharedObjType& shared, cweeThreadedList<u64>& bestPolicy, u64 bestPerformance) -> bool {
                auto& i = shared.results.Alloc();
                i.bestPolicy = bestPolicy;
                i.bestPerformance = bestPerformance;
                if (shared.results.Num() <= 10) return false; // guarrantee at least 10 iterations
                else {
                    // ensure the most recent 5 iterations are better than the 5 before them.
                    int i = shared.results.Num() - 1;
                    float perf_new = 0;
                    float perf_old = 0;
                    int count = 0;
                    for (; i >= (shared.results.Num() - 5); i--) {
                        cweeMath::rollingAverageRef(perf_new, shared.results[i].bestPerformance, count);
                    }
                    count = 0;
                    for (; i >= 0 && i >= (shared.results.Num() - 10); i--) {
                        cweeMath::rollingAverageRef(perf_old, shared.results[i].bestPerformance, count);
                    }

                    // ensure that we are improving. 
                    if (cweeMath::Fabs(perf_new - perf_old) <= cweeMath::Fabs(bestPerformance * eps)) {
                        shared.numIterationsFailedImprovement++;
                        if (shared.numIterationsFailedImprovement > (maxIterations / 10)) // (maxIterations / 10)) // 20 iterations processed and we saw no further improvement. Unlikely to see improvement with another 20.                         
                            return true;
                        else
                            return false;
                    }
                    shared.numIterationsFailedImprovement = 0;
                    return false; // we have not flat-lined. continue.
                }
            };

            cweeSharedPtr<sharedObjType> shared = make_cwee_shared<sharedObjType>();
            cweeJob toAwait;
            if (minimize) {
                switch (optimizationType.Hash()) {
                case cweeStr::Hash("Random"): {
                    Random_OptimizationManagementTool<true> ramt(numDimensions, ::Min<int>(128, numDimensions * 16));
                    ramt.lower_constraints() = lowerBound;
                    ramt.upper_constraints() = upperBound;
                    toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations);
                    break;
                }
                case cweeStr::Hash("Genetic"): {
                    Genetic_OptimizationManagementTool<true> ramt(numDimensions, ::Min<int>(128, numDimensions * 16));
                    ramt.lower_constraints() = lowerBound;
                    ramt.upper_constraints() = upperBound;
                    toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations);
                    break;
                }
                case cweeStr::Hash("PSO"): {
                    ParticleSwarm_OptimizationManagementTool<true> ramt(numDimensions, ::Min<int>(128, numDimensions * 16));
                    ramt.lower_constraints() = lowerBound;
                    ramt.upper_constraints() = upperBound;
                    toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations);
                    break;
                }
                }
            }
            else {
                switch (optimizationType.Hash()) {
                case cweeStr::Hash("Random"): {
                    Random_OptimizationManagementTool<false> ramt(numDimensions, ::Min<int>(128, numDimensions * 16));
                    ramt.lower_constraints() = lowerBound;
                    ramt.upper_constraints() = upperBound;
                    toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations);
                    break;
                }
                case cweeStr::Hash("Genetic"): {
                    Genetic_OptimizationManagementTool<false> ramt(numDimensions, ::Min<int>(128, numDimensions * 16));
                    ramt.lower_constraints() = lowerBound;
                    ramt.upper_constraints() = upperBound;
                    toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations);
                    break;
                }
                case cweeStr::Hash("PSO"): {
                    ParticleSwarm_OptimizationManagementTool<false> ramt(numDimensions, ::Min<int>(128, numDimensions * 16));
                    ramt.lower_constraints() = lowerBound;
                    ramt.upper_constraints() = upperBound;
                    toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations);
                    break;
                }
                }
            }
#ifdef ReturnFutureObjFromOptimizations
            cweeAny p = make_cwee_shared<std::promise<chaiscript::Boxed_Value>>(new std::promise<chaiscript::Boxed_Value>());
            auto awaiter = toAwait.ContinueWith(cweeJob([](std::promise<chaiscript::Boxed_Value>& promise, sharedObjType* results) {
                chaiscript::Boxed_Value toReturn;
                try {
                    chaiscript::small_vector< chaiscript::Boxed_Value > bv_final;
                    {
                        auto& bestPolicy = results->results[results->results.Num() - 1].bestPolicy;
                        for (auto& v_final : bestPolicy) { bv_final.push_back(chaiscript::var((double)v_final)); }
                    }
                    toReturn = chaiscript::var(bv_final);
                }
                catch (...) {}
                promise.set_value(toReturn);
            }, p, shared));

            return FutureObj(
                p, /* promise container */
                p.cast< std::promise<chaiscript::Boxed_Value>& >().get_future(), /* future container */
                awaiter /* job task */
            );
#else
            toAwait.AwaitAll();
            chaiscript::small_vector< chaiscript::Boxed_Value > bv_final;
            try {               
                auto& bestPolicy = shared->results[shared->results.Num() - 1].bestPolicy;
                for (auto& v_final : bestPolicy) { bv_final.push_back(chaiscript::var((double)v_final)); }                                
            } catch (...) {}
            return bv_final;
#endif
        };

        static void		AppendToScriptingLanguage(Module& scriptingLanguage) {
            scriptingLanguage.add(chaiscript::fun(&ChaiscriptOptimizations::OptimizeFunction, {"minimize", "per_sample_func", "lowBoundVector", "highBoundVector", "OptimizationType" }), "OptimizeFunction");

            if (1) {
                scriptingLanguage.eval(R"(
                    // example: Minimize(fun[](Vector x){ return x[0]*x[1]+x[0]-x[1]+100.0; }, [-100.0,-100.0], [100.0,100.0]);
			        def Minimize(Function per_sample, Vector lowBound, Vector highBound){   
                        var low_double = Vector(); for (x:lowBound){ low_double.push_back_ref(double(x)); }                    
                        var hih_double = Vector(); for (x:highBound){ hih_double.push_back_ref(double(x)); }
				        return OptimizeFunction(true, fun[per_sample](Vector x){ return double(per_sample(x)); }, low_double, hih_double, "Genetic");
			        };
			        def Maximize(Function per_sample, Vector lowBound, Vector highBound){
                        var low_double = Vector(); for (x:lowBound){ low_double.push_back_ref(double(x)); }                    
                        var hih_double = Vector(); for (x:highBound){ hih_double.push_back_ref(double(x)); }
				        return OptimizeFunction(false, fun[per_sample](Vector x){ return double(per_sample(x)); }, low_double, hih_double, "Genetic");
			        };
			        def Minimize(Function per_sample, Vector lowBound, Vector highBound, string OptType){   
                        var low_double = Vector(); for (x:lowBound){ low_double.push_back_ref(double(x)); }                    
                        var hih_double = Vector(); for (x:highBound){ hih_double.push_back_ref(double(x)); }
				        return OptimizeFunction(true, fun[per_sample](Vector x){ return double(per_sample(x)); }, low_double, hih_double, OptType);
			        };
			        def Maximize(Function per_sample, Vector lowBound, Vector highBound, string OptType){
                        var low_double = Vector(); for (x:lowBound){ low_double.push_back_ref(double(x)); }                    
                        var hih_double = Vector(); for (x:highBound){ hih_double.push_back_ref(double(x)); }
				        return OptimizeFunction(false, fun[per_sample](Vector x){ return double(per_sample(x)); }, low_double, hih_double, OptType);
			        };
		        )");
#ifndef ReturnFutureObjFromOptimizations
                scriptingLanguage.eval(R"(
                    // example: Minimize(fun[](Vector x){ return x[0]*x[1]+x[0]-x[1]+100.0; }, [-100.0,-100.0], [100.0,100.0]);
			        def MinimizeAsync(Function per_sample, Vector lowBound, Vector highBound){   
                        return Async(fun[per_sample, lowBound, highBound](){
                            return Minimize(per_sample, lowBound, highBound);
                        });
			        };
			        def MaximizeAsync(Function per_sample, Vector lowBound, Vector highBound){
                        return Async(fun[per_sample, lowBound, highBound](){
                            return Maximize(per_sample, lowBound, highBound);
                        });
			        };
			        def MinimizeAsync(Function per_sample, Vector lowBound, Vector highBound, string OptType){   
                        return Async(fun[per_sample, lowBound, highBound, OptType](){
                            return Minimize(per_sample, lowBound, highBound, OptType);
                        });
			        };
			        def MaximizeAsync(Function per_sample, Vector lowBound, Vector highBound, string OptType){
                        return Async(fun[per_sample, lowBound, highBound, OptType](){
                            return Maximize(per_sample, lowBound, highBound, OptType);
                        });
			        };
		        )");
#endif
            }
        };
    };
#undef OPT_FUNC_DIM
};

#define AddBasicClassTemplate(Type) { \
                    lib->add(chaiscript::user_type<Type>(), #Type); \
                    lib->add(chaiscript::constructor<Type()>(), #Type); \
                    lib->add(chaiscript::constructor<Type(const Type&)>(), #Type); \
                    lib->add(chaiscript::fun([](Type& a, const Type& b)->Type& { a = b; return a; }), "="); \
                }
#define AddBasicClassMember(Type, Member) { lib->add(chaiscript::fun(&##Type::##Member), #Member); }
#define AddNamespacedClassTemplate(Namespace, Type) { \
                    lib->add(chaiscript::user_type<##Namespace::##Type>(), #Type); \
                    lib->add(chaiscript::constructor<##Namespace::##Type()>(), #Type); \
                    lib->add(chaiscript::constructor<##Namespace::##Type(##Namespace::##Type const &)>(), #Type); \
                    lib->add(chaiscript::fun([](##Namespace::##Type& a, ##Namespace::##Type const& b)->##Namespace::##Type& { a = b; return a; }), "="); \
                }
#define AddNamespacedClassTemplate_SupportSharedPtr(Namespace, Type) { \
                    lib->add(chaiscript::user_type<##Namespace::##Type>(), #Type); \
                    lib->add(chaiscript::constructor<##Namespace::##Type()>(), #Type); \
                    lib->add(chaiscript::constructor<##Namespace::##Type(##Namespace::##Type const &)>(), #Type); \
                    lib->add(chaiscript::fun([](##Namespace::##Type& a, ##Namespace::##Type const& b)->##Namespace::##Type& { a = b; return a; }), "="); \
                    \
                    lib->add(chaiscript::user_type<cweeSharedPtr<##Namespace::##Type>>(), (cweeStr(#Type) + cweeStr("_Shared")).c_str()); \
                    lib->add(chaiscript::constructor<cweeSharedPtr<##Namespace::##Type>(cweeSharedPtr<##Namespace::##Type> const &)>(), (cweeStr(#Type) + cweeStr("_Shared")).c_str()); \
                    lib->add(chaiscript::fun([](cweeSharedPtr<##Namespace::##Type>& a, cweeSharedPtr<##Namespace::##Type> const& b)->cweeSharedPtr<##Namespace::##Type>& { a = b; return a; }), "="); \
                    lib->add(chaiscript::constructor<bool(cweeSharedPtr<##Namespace::##Type> const &)>(), "bool"); \
                    lib->add(chaiscript::type_conversion<cweeSharedPtr<##Namespace::##Type>, bool>([](const cweeSharedPtr<##Namespace::##Type>& t_bt)->bool { return (bool)t_bt; }, nullptr)); \
                }


#define AddSharedPtrClass(Namespace, BaseType) { \
                    lib->add(chaiscript::user_type<cweeSharedPtr<##Namespace::##BaseType>>(), #BaseType); \
                    lib->add(chaiscript::fun([]()->cweeSharedPtr<##Namespace::##BaseType> { return make_cwee_shared<##Namespace::##BaseType>(); }), #BaseType); \
                    lib->add(chaiscript::constructor<cweeSharedPtr<##Namespace::##BaseType>(cweeSharedPtr<##Namespace::##BaseType> const &)>(), #BaseType); \
                    lib->add(chaiscript::fun([](cweeSharedPtr<##Namespace::##BaseType>& a, cweeSharedPtr<##Namespace::##BaseType> const& b)->cweeSharedPtr<##Namespace::##BaseType>& { a = b; return a; }), "="); \
                    lib->add(chaiscript::constructor<bool(cweeSharedPtr<##Namespace::##BaseType> const &)>(), "bool"); \
                    lib->add(chaiscript::type_conversion<cweeSharedPtr<##Namespace::##BaseType>, bool>([](const cweeSharedPtr<##Namespace::##BaseType>& t_bt)->bool { return (bool)t_bt; }, nullptr)); \
                }
#define AddSharedPtrClassMember(Namespace, BaseType, Member) { \
                    lib->add(chaiscript::fun([](cweeSharedPtr<##Namespace::##BaseType>& a) -> decltype(##Namespace::##BaseType::##Member)& { if (a) return a->Member; else throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); }), #Member); \
                }
#define AddSharedPtrClassMember_SpecializedName(Namespace, BaseType, Member, newName) { \
                    lib->add(chaiscript::fun([](cweeSharedPtr<##Namespace::##BaseType>& a) -> decltype(##Namespace::##BaseType::##Member)& { if (a) return a->Member; else throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); }), #newName); \
                }
#define AddSharedPtrClassFunction(Namespace, BaseType, Function) { \
                    lib->add(chaiscript::fun([](cweeSharedPtr<##Namespace::##BaseType>& a) { if (a) return a->Function(); else throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); }), #Function); \
                }





#define AddNamespacedClassMember(Namespace, Type, Member) { lib->add(chaiscript::fun(&##Namespace::##Type::##Member), #Member); }
#define AddNamespacedClassMember_SupportSharedPtr(Namespace, Type, Member) {  \
        lib->add(chaiscript::fun(&##Namespace::##Type::##Member), #Member); \
        lib->add(chaiscript::fun([](cweeSharedPtr<##Namespace::##Type>& a) -> decltype(##Namespace::##Type::##Member)& { if (a) return a->Member; else throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); }), #Member); \
}
#define AddNamespacedClassMember_SupportSharedPtr_SpecializedName(Namespace, Type, Member, newName) {  \
        lib->add(chaiscript::fun(&##Namespace::##Type::##Member), #newName); \
        lib->add(chaiscript::fun([](cweeSharedPtr<##Namespace::##Type>& a) -> decltype(##Namespace::##Type::##Member)& { if (a) return a->Member; else throw(chaiscript::exception::eval_error("Cannot access a member of a null (empty) shared object.")); }), #newName); \
}
// #define AddBasicClassMember_FromSharedPtr(Namespace, Type, Member) { lib->add(chaiscript::fun(&##Namespace::##Type::##Member), #Member); }
#define AddNamespacedClassFunction(Namespace, Type, Member) { lib->add(chaiscript::fun(&##Namespace::##Type::##Member), #Member); }
#define AddNamespacedClassFunction_SupportSharedPtr(Namespace, Type, Function) {  \
        lib->add(chaiscript::fun(&##Namespace::##Type::##Function), #Function); \
        lib->add(chaiscript::fun([](cweeSharedPtr<##Namespace::##Type>& a){ if (a) return a->Function(); else throw(chaiscript::exception::eval_error("Cannot access a function of a null (empty) shared object.")); }), #Function); \
}

#define ChaiscriptConstructor(From, To) {\
lib->add(chaiscript::constructor<To(const From&)>(), #To);\
lib->add(chaiscript::fun([](To& a, const From& b)->To& { a = To(b); return a; }), "=");\
lib->add(chaiscript::type_conversion<From, To>([](const From& t_bt)->To { return To(t_bt); }, nullptr)); }
#define ADD_BETTER_ENUM_TO_SCRIPT_ENGINE(Type, TypeName) {   \
            lib->add(chaiscript::user_type<Type>(), #TypeName);    \
            lib->add(chaiscript::constructor<Type()>(), #TypeName);    \
            lib->add(chaiscript::constructor<Type(const Type&)>(), #TypeName);    \
            lib->add(chaiscript::fun([](Type& a, Type const& b) { a = b; return a; }), "=");    \
            lib->add(chaiscript::type_conversion<const Type, int>([](const Type& t_bt)->int { return (int)t_bt; }, nullptr));    \
            lib->add(chaiscript::type_conversion<const int, Type>([](int t_bt)->Type { return Type::_from_integral(t_bt); }, nullptr));   \
            lib->add(chaiscript::type_conversion<const std::string, Type>([](const std::string& t_bt) {   \
                cweeList<cweeStr> strings; for (auto str : Type::_names()) { strings.Append(str); }   \
                auto bestMatch = cweeStr(t_bt.c_str()).BestMatch(strings); return Type::_from_string(bestMatch.c_str()); }, nullptr));   \
            lib->add(chaiscript::type_conversion<const Type, std::string>([](const Type& t_bt) { std::string a = t_bt.ToString(); return a; }, nullptr));   \
            lib->add(chaiscript::fun([](Type& a, const std::string& b) {   \
                cweeList<cweeStr> strings; for (auto str : Type::_names()) { strings.Append(str); }   \
                auto bestMatch = cweeStr(b.c_str()).BestMatch(strings); a = Type::_from_string(bestMatch.c_str()); return a; }), "=");   \
            lib->add(chaiscript::fun([](std::string& a, const Type& b) { a = b.ToString(); return a; }), "=");   \
            int i = 0; for (auto& enumThing : Type::_values()) {   \
                cweeStr nm = enumThing.ToString();   \
                nm.ReduceSpaces(); nm.ReplaceInline(" ", ""); nm.ReplaceInline("$", "");   \
                nm.ReplaceInline("&", ""); nm.ReplaceInline("*", ""); nm.ReplaceInline("%", "");   \
                nm.ReplaceInline("#", ""); nm.ReplaceInline("@", ""); nm.ReplaceInline("!", "");   \
                nm.ReplaceInline("+", ""); nm.ReplaceInline("=", ""); nm.ReplaceInline("-", "");   \
                nm.ReplaceInline("`", ""); nm.ReplaceInline("\"", ""); nm.ReplaceInline("'", "");   \
                nm.ReplaceInline("[", ""); nm.ReplaceInline("]", ""); nm.ReplaceInline("(", "");   \
                nm.ReplaceInline(")", ""); nm.ReplaceInline("/", ""); nm.ReplaceInline("\\", "");   \
                nm.ReplaceInline("^", ""); nm.ReplaceInline("?", ""); nm.ReplaceInline("²", "");   \
                cweeStr addEnumFunc = cweeStr::printf("def %s::%s() { return %s(%i); }", cweeStr(#TypeName).c_str(), nm.c_str(), cweeStr(#TypeName).c_str(), i);   \
                try { lib->eval(addEnumFunc.c_str()); }   \
                catch (...) { fileSystem->submitToast(cweeStr("Error adding Enum ") + cweeStr(nm.c_str()), "With Enum class " + cweeStr(#TypeName)); } i++;   \
            }   \
        }

#define AddUnit_t(Type)     { using namespace cwee_units; using namespace cweeUnitValues;  \
                    lib->add(chaiscript::user_type<Type>(), #Type);  \
                    lib->add(chaiscript::constructor<Type()>(), #Type);  \
                    lib->add(chaiscript::constructor<Type(const Type&)>(), #Type);   \
                    lib->add(chaiscript::fun([](Type& a, const Type& b) { a = b; return a; }), "=");   \
                    lib->add(chaiscript::constructor<Type(double)>(), #Type);   \
                    lib->add(chaiscript::constructor<Type(float)>(), #Type);   \
                    lib->add(chaiscript::constructor<Type(int)>(), #Type);   \
                    lib->add(fun([](Type& a, const int& b)->Type& { a = b; return a; }), "=");   \
                    lib->add(fun([](Type& a, const double& b)->Type& { a = b; return a; }), "=");   \
                    lib->add(fun([](Type& a, const float& b)->Type& { a = b; return a; }), "=");   \
                    lib->add(fun([](Type& a, const u64& b)->Type& { a = b; return a; }), "=");   \
                    lib->add(fun([](Type& a, const long& b)->Type& { a = b; return a; }), "=");   \
                    lib->add(type_conversion<Type, int>([](const Type& a)->int { return a(); }, nullptr));  \
                    lib->add(type_conversion<Type, double>([](const Type& a)->double { return a(); }, nullptr));  \
                    lib->add(type_conversion<Type, float>([](const Type& a)->float { return a(); }, nullptr));  \
                    lib->add(type_conversion<Type, u64>([](const Type& a)->u64 { return a(); }, nullptr));  \
                    lib->add(type_conversion<Type, long>([](const Type& a)->long { return a(); }, nullptr));  \
                    lib->add(chaiscript::fun([](Type const& a) -> int { return a(); }), "int");   \
                    lib->add(chaiscript::fun([](Type const& a) -> double { return a(); }), "double");   \
                    lib->add(chaiscript::fun([](Type const& a) -> float { return a(); }), "float");   \
                    lib->add(chaiscript::fun([](Type const& a) -> u64 { return a(); }), "u64");   \
                    lib->add(chaiscript::fun([](Type const& a) -> long { return a(); }), "long");   \
                    lib->add(fun([](int& a, const Type& b)->int& { a = b(); return a; }), "=");   \
                    lib->add(fun([](double& a, const Type& b)->double& { a = b(); return a; }), "=");   \
                    lib->add(fun([](float& a, const Type& b)->float& { a = b(); return a; }), "=");   \
                    lib->add(fun([](u64& a, const Type& b)->u64& { a = b(); return a; }), "=");   \
                    lib->add(fun([](long& a, const Type& b)->long& { a = b(); return a; }), "=");   \
                    lib->add(type_conversion<Type, unit_value>([](const Type& a)->unit_value { return unit_value(a); }, nullptr)); \
                    lib->add(type_conversion<unit_value, Type>([](const unit_value& a)->Type { unit_value temp = unit_value(Type()); temp = a; Type out = temp(); return out; }, nullptr)); \
                    lib->add(chaiscript::fun([](Type& a, const unit_value& b)->Type& { unit_value temp = unit_value(Type()); temp = b; a = temp(); return a; }), "=");   \
                    lib->add(chaiscript::fun([](unit_value& a, const Type& b)->unit_value& { a = unit_value(b); return a; }), "=");   \
                    lib->add(chaiscript::fun([](const Type& a, const Type& b) { return a + b; }), "+");   \
                    lib->add(chaiscript::fun([](const Type& a, const Type& b) { return a - b; }), "-");   \
                    lib->add(chaiscript::fun([](Type& a, const Type& b) -> Type& { a += b; return a; }), "+=");   \
                    lib->add(chaiscript::fun([](Type& a, const Type& b) -> Type& { a -= b; return a; }), "-="); \
                    lib->add(chaiscript::fun([](const Type& a)->cweeStr { \
                            using namespace cweeUnitValues; \
                            unit_value t = a; \
                            return t.ToString(); \
                    }), "to_string"); \
                    lib->add(chaiscript::fun([](const Type& a)->std::string { \
                        using namespace cweeUnitValues; \
                        unit_value t = a; \
                        return t.Abbreviation(); \
                    }), "Abbreviation"); \
                }

#define STR_FUNC( thing ) cweeStr(#thing)
#define DEF_DECLARE_PAIR( ValueTypeAClass, ValueTypeBClass ) \
                { \
                    cweeStr pairName = (cweeStr("pair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
                    chaiscript::bootstrap::standard_library::pair_type<std::pair<ValueTypeAClass, ValueTypeBClass>>(pairName.c_str(), *lib); \
                    lib->add(chaiscript::type_conversion<std::pair<ValueTypeAClass, ValueTypeBClass>, std::string>([](const std::pair<ValueTypeAClass, ValueTypeBClass>& t_bt) { return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(t_bt.first).c_str(), cweeStr::ToString<ValueTypeBClass>(t_bt.second).c_str()).c_str()); }, nullptr)); \
                    lib->add(chaiscript::type_conversion<std::pair<ValueTypeAClass, ValueTypeBClass>, std::pair<Boxed_Value, Boxed_Value>>([](const std::pair<ValueTypeAClass, ValueTypeBClass>& t_bt) { return std::pair<Boxed_Value, Boxed_Value>(var((ValueTypeAClass)(t_bt.first)), var((ValueTypeBClass)(t_bt.second))); }, nullptr)); \
                    lib->add(chaiscript::fun([](const std::pair<ValueTypeAClass, ValueTypeBClass>& t_bt) { return std::pair<Boxed_Value, Boxed_Value>(var((ValueTypeAClass)(t_bt.first)), var((ValueTypeBClass)(t_bt.second))); }), "Pair"); \
                    lib->add(chaiscript::fun([](std::string& a, const std::pair<ValueTypeAClass, ValueTypeBClass>& b) {	a = std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); return a; }), "="); \
                    lib->add(chaiscript::fun([](const std::pair<ValueTypeAClass, ValueTypeBClass>& b) {	return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); }), "to_string"); \
                } \
                { \
                    cweeStr pairName = (cweeStr("cweepair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
                    chaiscript::bootstrap::standard_library::pair_type<cweePair<ValueTypeAClass, ValueTypeBClass>>(pairName.c_str(), *lib); \
                    lib->add(chaiscript::type_conversion<cweePair<ValueTypeAClass, ValueTypeBClass>, std::string>([](const cweePair<ValueTypeAClass, ValueTypeBClass>& t_bt) { return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(t_bt.first).c_str(), cweeStr::ToString<ValueTypeBClass>(t_bt.second).c_str()).c_str()); }, nullptr)); \
                    lib->add(chaiscript::type_conversion<cweePair<ValueTypeAClass, ValueTypeBClass>, std::pair<Boxed_Value, Boxed_Value>>([](const cweePair<ValueTypeAClass, ValueTypeBClass>& t_bt) { return std::pair<Boxed_Value, Boxed_Value>(var((ValueTypeAClass)(t_bt.first)), var((ValueTypeBClass)(t_bt.second))); }, nullptr)); \
                    lib->add(chaiscript::fun([](const cweePair<ValueTypeAClass, ValueTypeBClass>& t_bt) { return std::pair<Boxed_Value, Boxed_Value>(var((ValueTypeAClass)(t_bt.first)), var((ValueTypeBClass)(t_bt.second))); }), "Pair"); \
                    lib->add(chaiscript::fun([](std::string& a, const cweePair<ValueTypeAClass, ValueTypeBClass>& b) {	a = std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); return a; }), "="); \
                    lib->add(chaiscript::fun([](const cweePair<ValueTypeAClass, ValueTypeBClass>& b) {	return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); }), "to_string"); \
                }

//#define DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE( ValueTypeClass ) \
//        if (true) { \
//            cweeStr vecName = (cweeStr("vector_") + STR_FUNC(ValueTypeClass)).ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
//            cweeStr cweeVecName = (cweeStr("cweevector_") + STR_FUNC(ValueTypeClass)).ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
//            lib->add(chaiscript::vector_conversion<std::vector<chaiscript::Boxed_Value>, std::vector<ValueTypeClass>>(nullptr)); \
//            lib->add(chaiscript::vector_conversion<std::vector<chaiscript::Boxed_Value>, cweeThreadedList<ValueTypeClass>>(nullptr)); \
//            lib->add(chaiscript::vector_conversion<cweeThreadedList<chaiscript::Boxed_Value>, std::vector<ValueTypeClass>>(nullptr)); \
//            lib->add(chaiscript::vector_conversion<cweeThreadedList<chaiscript::Boxed_Value>, cweeThreadedList<ValueTypeClass>>(nullptr)); \
//            chaiscript::bootstrap::standard_library::vector_type<std::vector<ValueTypeClass>>(vecName.c_str(), *lib);  \
//            chaiscript::bootstrap::standard_library::vector_type<cweeThreadedList<ValueTypeClass>>(cweeVecName.c_str(), *lib); \
//            lib->add(chaiscript::constructor<cweeThreadedList<ValueTypeClass>(const std::vector<chaiscript::Boxed_Value>&)>(), cweeVecName.c_str());	\
//            lib->add(chaiscript::constructor<cweeThreadedList<ValueTypeClass>(const cweeThreadedList<chaiscript::Boxed_Value>&)>(), cweeVecName.c_str());	\
//            lib->add(chaiscript::constructor<std::vector<ValueTypeClass>(const cweeThreadedList<chaiscript::Boxed_Value>&)>(), vecName.c_str()); \
//            lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, const cweeThreadedList<ValueTypeClass>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
//            lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, const std::vector<ValueTypeClass>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<ValueTypeClass>)b; return t_ti; }), "=");	\
//            lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, const cweeThreadedList<chaiscript::Boxed_Value>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
//            lib->add(chaiscript::fun([](std::vector<ValueTypeClass>& t_ti, cweeThreadedList<ValueTypeClass>& b) { t_ti = (std::vector<ValueTypeClass>)b; return t_ti; }), "=");	\
//            lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, cweeThreadedList<ValueTypeClass>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
//            lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, std::vector<ValueTypeClass>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
//            lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, std::vector<chaiscript::Boxed_Value>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
//            lib->add(chaiscript::fun([](cweeThreadedList<ValueTypeClass>& t_ti, std::vector<ValueTypeClass>& b) { t_ti = (cweeThreadedList<ValueTypeClass>)b; return t_ti; }), "=");	\
//            lib->add(chaiscript::fun([](cweeThreadedList<ValueTypeClass>& t_ti, cweeThreadedList<ValueTypeClass>& b) { t_ti = b; return t_ti; }), "=");	\
//            lib->add(chaiscript::fun([](std::vector<ValueTypeClass>& t_ti, std::vector<ValueTypeClass>& b) { t_ti = b; return t_ti; }), "=");	\
//            lib->add(chaiscript::type_conversion<cweeThreadedList<ValueTypeClass>&, std::vector<ValueTypeClass>&>([](const cweeThreadedList<ValueTypeClass>& t_bt) { std::vector<ValueTypeClass> out; out = (std::vector<ValueTypeClass>)t_bt; return out; }, nullptr));	\
//            lib->add(chaiscript::type_conversion<std::vector<ValueTypeClass>&, cweeThreadedList<ValueTypeClass>&>([](const std::vector<ValueTypeClass>& t_bt) { cweeThreadedList<ValueTypeClass> out; out = (cweeThreadedList<ValueTypeClass>)t_bt; return out; }, nullptr));	\
//            lib->add(chaiscript::type_conversion<cweeThreadedList<ValueTypeClass>&, cweeThreadedList<chaiscript::Boxed_Value>&>([](const cweeThreadedList<ValueTypeClass>& t_bt) { cweeThreadedList<chaiscript::Boxed_Value> out; out = (cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
//            lib->add(chaiscript::type_conversion<std::vector<ValueTypeClass>&, cweeThreadedList<chaiscript::Boxed_Value>&>([](const std::vector<ValueTypeClass>& t_bt) { cweeThreadedList<chaiscript::Boxed_Value> out; out = (cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
//            lib->add(chaiscript::type_conversion<cweeThreadedList<ValueTypeClass>&, std::vector<chaiscript::Boxed_Value>&>([](const cweeThreadedList<ValueTypeClass>& t_bt) { std::vector<chaiscript::Boxed_Value> out; out = (std::vector<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
//            lib->add(chaiscript::type_conversion<std::vector<ValueTypeClass>&, std::vector<chaiscript::Boxed_Value>&>([](const std::vector<ValueTypeClass>& t_bt) { std::vector<chaiscript::Boxed_Value> out; out = (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
//            lib->add(chaiscript::constructor<cweeThreadedList<chaiscript::Boxed_Value>(const std::vector<ValueTypeClass>&)>(), "cweeVector");	\
//            lib->add(chaiscript::constructor<cweeThreadedList<chaiscript::Boxed_Value>(const cweeThreadedList<ValueTypeClass>&)>(), "cweeVector");	\
//        }
#define MACRO_COMMA ,
#define DEF_DECLARE_STD_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE( ValueTypeClass ) \
        if (true) { \
            cweeStr vecName = (cweeStr("vector_") + STR_FUNC(ValueTypeClass)).ReplaceInline(",", "_").ReplaceInline("<", "_").ReplaceInline(">", "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline("__", "_").ReplaceInline(" ", ""); \
            /* Decl the type */ chaiscript::bootstrap::standard_library::vector_type<std::vector<ValueTypeClass>>(vecName.c_str(), *lib);  \
            /* Vector -> custom */ lib->add(chaiscript::vector_conversion<std::vector<chaiscript::Boxed_Value>, std::vector<ValueTypeClass>>(nullptr)); \
            /* Vector x = custom(); */ lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, const std::vector<ValueTypeClass>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<chaiscript::Boxed_Value>)(cweeThreadedList<ValueTypeClass>)b; return t_ti; }), "=");	\
            /* custom x = custom(); */ lib->add(chaiscript::fun([](std::vector<ValueTypeClass>& t_ti, std::vector<ValueTypeClass>& b) { t_ti = b; return t_ti; }), "=");	\
            /* custom x = Vector(); */ lib->add(chaiscript::fun([](std::vector<ValueTypeClass>& t_ti, std::vector<chaiscript::Boxed_Value>& b) { t_ti = (std::vector<ValueTypeClass>)(cweeThreadedList<ValueTypeClass>)(cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            /* Vector x = Vector(custom()); */ lib->add(chaiscript::type_conversion<std::vector<ValueTypeClass>&, std::vector<chaiscript::Boxed_Value>&>([](const std::vector<ValueTypeClass>& t_bt) { std::vector<chaiscript::Boxed_Value> out; out = (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<chaiscript::Boxed_Value>)(cweeThreadedList<ValueTypeClass>)t_bt; return out; }, nullptr));	\
        }
#define DEF_DECLARE_CWEE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE( ValueTypeClass ) \
        if (true) { \
            cweeStr vecName = (cweeStr("cweevector_") + STR_FUNC(ValueTypeClass)).ReplaceInline(",", "_").ReplaceInline("<", "_").ReplaceInline(">", "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline("__", "_").ReplaceInline(" ", ""); \
            /* Decl the type */ chaiscript::bootstrap::standard_library::vector_type<cweeThreadedList<ValueTypeClass>>(vecName.c_str(), *lib);  \
            /* Vector -> custom */ lib->add(chaiscript::vector_conversion<std::vector<chaiscript::Boxed_Value>, cweeThreadedList<ValueTypeClass>>(nullptr)); \
            /* Vector x = custom(); */ lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, const cweeThreadedList<ValueTypeClass>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            /* custom x = custom(); */ lib->add(chaiscript::fun([](cweeThreadedList<ValueTypeClass>& t_ti, cweeThreadedList<ValueTypeClass>& b) { t_ti = b; return t_ti; }), "=");	\
            /* custom x = Vector(); */ lib->add(chaiscript::fun([](cweeThreadedList<ValueTypeClass>& t_ti, std::vector<chaiscript::Boxed_Value>& b) { t_ti = (cweeThreadedList<ValueTypeClass>)(cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            /* Vector x = Vector(custom()); */ lib->add(chaiscript::type_conversion<cweeThreadedList<ValueTypeClass>&, std::vector<chaiscript::Boxed_Value>&>([](const cweeThreadedList<ValueTypeClass>& t_bt) { return (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<chaiscript::Boxed_Value>)t_bt; }, nullptr));	\
        }
#define DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE( ValueTypeClass ) \
        if (true) { \
            DEF_DECLARE_STD_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(ValueTypeClass); \
            DEF_DECLARE_CWEE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(ValueTypeClass); \
        }
#define DEF_DECLARE_STD_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE( ValueTypeAClass, ValueTypeBClass ) \
        if (true){ \
            cweeStr pairName = (cweeStr("pair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "_").ReplaceInline(",", "_").ReplaceInline("<", "_").ReplaceInline(">", "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline("__", "_").ReplaceInline(" ", ""); \
            /* Decl the type */ chaiscript::bootstrap::standard_library::pair_type<std::pair<ValueTypeAClass, ValueTypeBClass>>(pairName.c_str(), *lib); \
            lib->eval(cweeStr::printf(R"(def to_string(%s v){ return "<${v.first}, ${v.second}>"; };)", pairName.c_str()).c_str()); \
            lib->eval(cweeStr::printf(R"(def `=`(string s, %s v){ s ="<${v.first}, ${v.second}>"; return s; };)", pairName.c_str()).c_str()); \
        }
#define DEF_DECLARE_CWEE_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE( ValueTypeAClass, ValueTypeBClass ) \
        if (true){ \
            cweeStr pairName = (cweeStr("cweepair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "_").ReplaceInline(",", "_").ReplaceInline("<", "_").ReplaceInline(">", "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline("__", "_").ReplaceInline(" ", ""); \
            /* Decl the type */ chaiscript::bootstrap::standard_library::pair_type<cweePair<ValueTypeAClass, ValueTypeBClass>>(pairName.c_str(), *lib); \
            lib->eval(cweeStr::printf(R"(def to_string(%s v){ return "<${v.first}, ${v.second}>"; };)", pairName.c_str()).c_str()); \
            lib->eval(cweeStr::printf(R"(def `=`(string s, %s v){ s ="<${v.first}, ${v.second}>"; return s; };)", pairName.c_str()).c_str()); \
        }
#define DEF_DECLARE_VECTOR_OF_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE( ValueTypeAClass, ValueTypeBClass )  \
        if (true){ \
            DEF_DECLARE_STD_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE(ValueTypeAClass, ValueTypeBClass);\
            DEF_DECLARE_CWEE_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE(ValueTypeAClass, ValueTypeBClass);\
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(std::pair<ValueTypeAClass MACRO_COMMA ValueTypeBClass>); \
            DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(cweePair<ValueTypeAClass MACRO_COMMA ValueTypeBClass>); \
        }

/*
#define DEF_DECLARE_VECTOR_OF_PAIR_WITH_SCRIPT_ENGINE_AND_MODULE( ValueTypeAClass, ValueTypeBClass )  \
        if (true) { \
            cweeStr vecName = (cweeStr("vector_pair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
            cweeStr cweeVecName = (cweeStr("cweevector_pair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
            cweeStr pairName = (cweeStr("pair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
            lib->add(chaiscript::vector_conversion<std::vector<chaiscript::Boxed_Value>, std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>>(nullptr)); \
            lib->add(chaiscript::vector_conversion<std::vector<chaiscript::Boxed_Value>, cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>>(nullptr)); \
            lib->add(chaiscript::vector_conversion<cweeThreadedList<chaiscript::Boxed_Value>, std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>>(nullptr)); \
            lib->add(chaiscript::vector_conversion<cweeThreadedList<chaiscript::Boxed_Value>, cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>>(nullptr)); \
            chaiscript::bootstrap::standard_library::vector_type<std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>>(vecName.c_str(), *lib); \
            chaiscript::bootstrap::standard_library::pair_type<std::pair<ValueTypeAClass, ValueTypeBClass>>(pairName.c_str(), *lib); \
            chaiscript::bootstrap::standard_library::vector_type<cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>>(cweeVecName.c_str(), *lib); \
            lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, cweeThreadedList<chaiscript::Boxed_Value>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_ti, cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, std::vector<chaiscript::Boxed_Value>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_ti, std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_ti, cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_ti, std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = b; return t_ti; }), "=");	\
            lib->add(chaiscript::type_conversion<cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>&, std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>&>([](const cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>> out; out = (std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>&, cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>&>([](const std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>> out; out = (cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>&, cweeThreadedList<chaiscript::Boxed_Value>&>([](const cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { cweeThreadedList<chaiscript::Boxed_Value> out; out = (cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>&, cweeThreadedList<chaiscript::Boxed_Value>&>([](const std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { cweeThreadedList<chaiscript::Boxed_Value> out; out = (cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>&, std::vector<chaiscript::Boxed_Value>&>([](const cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { std::vector<chaiscript::Boxed_Value> out; out = (std::vector<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>&, std::vector<chaiscript::Boxed_Value>&>([](const std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { std::vector<chaiscript::Boxed_Value> out; out = (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::constructor<cweeThreadedList<chaiscript::Boxed_Value>(const std::vector<std::pair<ValueTypeAClass, ValueTypeBClass>>&)>(), "cweeVector");	\
            lib->add(chaiscript::constructor<cweeThreadedList<chaiscript::Boxed_Value>(const cweeThreadedList<std::pair<ValueTypeAClass, ValueTypeBClass>>&)>(), "cweeVector");	\
            lib->add(chaiscript::type_conversion<std::pair<ValueTypeAClass, ValueTypeBClass>, std::string>([](const std::pair<ValueTypeAClass, ValueTypeBClass>& t_bt) { return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(t_bt.first).c_str(), cweeStr::ToString<ValueTypeBClass>(t_bt.second).c_str()).c_str()); }, nullptr)); \
            lib->add(chaiscript::fun([](std::string& a, const std::pair<ValueTypeAClass, ValueTypeBClass>& b) {	a = std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); return a; }), "="); \
            lib->add(chaiscript::fun([](const std::pair<ValueTypeAClass, ValueTypeBClass>& b) {	return std::string(cweeStr::printf("%s|%s", cweeStr::ToString<ValueTypeAClass>(b.first).c_str(), cweeStr::ToString<ValueTypeBClass>(b.second).c_str()).c_str()); }), "to_string"); \
        } \
        if (true) { \
            cweeStr vecName = (cweeStr("vector_cweepair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
            cweeStr cweeVecName = (cweeStr("cweevector_cweepair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
            cweeStr pairName = (cweeStr("cweepair_") + STR_FUNC(ValueTypeAClass) + "_" + STR_FUNC(ValueTypeBClass) + "_").ReplaceInline("std::", "").ReplaceInline("::", "").ReplaceInline(" ", ""); \
            lib->add(chaiscript::vector_conversion<std::vector<chaiscript::Boxed_Value>, std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>>(nullptr)); \
            lib->add(chaiscript::vector_conversion<std::vector<chaiscript::Boxed_Value>, cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>>(nullptr)); \
            lib->add(chaiscript::vector_conversion<cweeThreadedList<chaiscript::Boxed_Value>, std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>>(nullptr)); \
            lib->add(chaiscript::vector_conversion<cweeThreadedList<chaiscript::Boxed_Value>, cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>>(nullptr)); \
            chaiscript::bootstrap::standard_library::vector_type<std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>>(vecName.c_str(), *lib); \
            chaiscript::bootstrap::standard_library::pair_type<cweePair<ValueTypeAClass, ValueTypeBClass>>(pairName.c_str(), *lib); \
            chaiscript::bootstrap::standard_library::vector_type<cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>>(cweeVecName.c_str(), *lib); \
            lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](std::vector<chaiscript::Boxed_Value>& t_ti, cweeThreadedList<chaiscript::Boxed_Value>& b) { t_ti = (std::vector<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_ti, cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<chaiscript::Boxed_Value>& t_ti, std::vector<chaiscript::Boxed_Value>& b) { t_ti = (cweeThreadedList<chaiscript::Boxed_Value>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_ti, std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = (cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>)b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_ti, cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = b; return t_ti; }), "=");	\
            lib->add(chaiscript::fun([](std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_ti, std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>& b) { t_ti = b; return t_ti; }), "=");	\
            lib->add(chaiscript::type_conversion<cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>&, std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>&>([](const cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>> out; out = (std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>&, cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>&>([](const std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>> out; out = (cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>&, cweeThreadedList<chaiscript::Boxed_Value>&>([](const cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { cweeThreadedList<chaiscript::Boxed_Value> out; out = (cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>&, cweeThreadedList<chaiscript::Boxed_Value>&>([](const std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { cweeThreadedList<chaiscript::Boxed_Value> out; out = (cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>&, std::vector<chaiscript::Boxed_Value>&>([](const cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { std::vector<chaiscript::Boxed_Value> out; out = (std::vector<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::type_conversion<std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>&, std::vector<chaiscript::Boxed_Value>&>([](const std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>& t_bt) { std::vector<chaiscript::Boxed_Value> out; out = (std::vector<chaiscript::Boxed_Value>)(cweeThreadedList<chaiscript::Boxed_Value>)t_bt; return out; }, nullptr));	\
            lib->add(chaiscript::constructor<cweeThreadedList<chaiscript::Boxed_Value>(const std::vector<cweePair<ValueTypeAClass, ValueTypeBClass>>&)>(), "cweeVector");	\
            lib->add(chaiscript::constructor<cweeThreadedList<chaiscript::Boxed_Value>(const cweeThreadedList<cweePair<ValueTypeAClass, ValueTypeBClass>>&)>(), "cweeVector");	\
            lib->eval(cweeStr::printf(R"(def to_string(%s v){ return "<${v.first}, ${v.second}>"; };)", pairName.c_str()).c_str()); \
            lib->eval(cweeStr::printf("def `=`(string s, %s v){ s = \"<${v.first}, ${v.second}>\"; return s; };", pairName.c_str()).c_str()); \
        }
 */

#define AddUnit(Type)\
                    lib->add(chaiscript::fun([]() -> unit_value { static Type F; return F; }), #Type); \
                    lib->add(chaiscript::fun([](unit_value const& a) -> unit_value { Type F; F = a; return F; }), #Type); \
                    lib->add(chaiscript::postfix((cweeStr("_") + cweeStr(Type::specialized_abbreviation())).c_str(), #Type));

#define AddUnitWithMetricPrefixes(Type)\
	                AddUnit(Type); \
	                AddUnit(femto ## Type); \
	                AddUnit(pico ## Type); \
	                AddUnit(nano ## Type); \
	                AddUnit(micro ## Type); \
	                AddUnit(milli ## Type); \
	                AddUnit(centi ## Type); \
	                AddUnit(deci ## Type); \
	                AddUnit(deca ## Type); \
	                AddUnit(hecto ## Type); \
	                AddUnit(kilo ## Type); \
	                AddUnit(mega ## Type); \
	                AddUnit(giga ## Type); \
	                AddUnit(tera ## Type); \
	                AddUnit(peta ## Type)
