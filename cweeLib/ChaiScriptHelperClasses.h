#ifndef __chaiScriptHelperClasses_H__
#define __chaiScriptHelperClasses_H__

#pragma hdrstop
#include "precompiled.h"

//class cweeGeopoint {
//public:
//	cweeGeopoint() {
//		longitude = chaiscript::Boxed_Value(0.0);
//		latitude = chaiscript::Boxed_Value(0.0);
//	};
//	cweeGeopoint(double Long, double Lat) {
//		longitude = chaiscript::Boxed_Value(Long);
//		latitude = chaiscript::Boxed_Value(Lat);
//	};
//public:
//	chaiscript::Boxed_Value longitude;
//	chaiscript::Boxed_Value latitude;
//
//};

#define AddMemberToScriptFromClass(Name) scriptingLanguage.add(chaiscript::fun(static_cast<decltype(Name)(ThisType::*)>(&ThisType::Name)), STR_FUNC(Name))
#define AddFuncToScriptFromClass(Name) scriptingLanguage.add(chaiscript::fun(&ThisType::Name), STR_FUNC(Name))

class cweeColor {
public:
	typedef cweeColor ThisType;
	static  std::string	ThisTypeName() { return "cweeColor"; };

	cweeColor() : R(128), G(128), B(128), A(255) {};
	cweeColor(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {};
	cweeColor(cweeColor const& r) : R(r.R), G(r.G), B(r.B), A(r.A) {};

	float R, G, B, A;

	cweeColor Lerp(const cweeColor& whenOne, float lerp) const {
		cweeColor out;

		out.R = whenOne.R * lerp + this->R * (1.0f - lerp);
		out.G = whenOne.G * lerp + this->G * (1.0f - lerp);
		out.B = whenOne.B * lerp + this->B * (1.0f - lerp);
		out.A = whenOne.A * lerp + this->A * (1.0f - lerp);

		return out;
	};

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(float, float, float, float)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::fun([](cweeColor& a) { return std::string(cweeStr::printf("[%s, %s, %s, %s]", cweeStr(a.R).c_str(), cweeStr(a.G).c_str(), cweeStr(a.B).c_str(), cweeStr(a.A).c_str()).c_str()); }), "to_string");

		AddMemberToScriptFromClass(R);
		AddMemberToScriptFromClass(G);
		AddMemberToScriptFromClass(B);
		AddMemberToScriptFromClass(A);
		scriptingLanguage.add(chaiscript::fun(&ThisType::Lerp), "Lerp");
		AddFuncToScriptFromClass(Lerp);

		scriptingLanguage.eval(R"(
			def `=`(cweeColor a, string  b){
				return fun[a,b](){
					a = App.ThemeColor(b);
					return a;
				}();				
			}
			
			
		)");
	};
};

class cweeMapIcon {
public:
	cweeMapIcon() : color(cweeColor()), size(12), longitude(0), latitude(0) {};
	cweeMapIcon(double Long, double Lat) : color(cweeColor()), size(12), longitude(Long), latitude(Lat) {};

public:
	cweeColor  color;
	double size;
	double longitude;
	double latitude;
	chaiscript::Boxed_Value tag;

};

class cweeMapPolyline {
public:
	cweeMapPolyline() : coordinates(cweeThreadedList<std::pair<double, double>>()), color(cweeColor()), dashed(false), thickness(2) {};
	void AddPoint(double X, double Y) {
		coordinates.Append(std::pair<double, double>(X, Y));
	};

public:
	cweeThreadedList<std::pair<double, double>> coordinates;
	cweeColor	color;
	bool		dashed;
	int			thickness;
	chaiscript::Boxed_Value tag;

};

class cweeMapBackground {
public:
	cweeMapBackground() : data(cweeInterpolatedMatrix<float>()), minColor(cweeColor()), maxColor(cweeColor()) {};
	cweeColor	ColorForPosition(double X, double Y) {
		float delta = (data.GetCurrentValue(X, Y) - data.GetMinValue()) / data.GetMaxValue(); // 0 to 1		
		return minColor.Lerp(maxColor, delta);
	};

public:
	cweeInterpolatedMatrix<float> data;
	cweeColor minColor;
	cweeColor maxColor;
	
};





#define AddFrameworkElementCharacteristicsToScriptFromClass() 	AddMemberToScriptFromClass(Version);\
AddMemberToScriptFromClass(UniqueName);\
scriptingLanguage.add(chaiscript::fun(&ThisType::Update), "Update");\
scriptingLanguage.add(chaiscript::fun(&ThisType::AddTask), "AddTask");\
scriptingLanguage.add(chaiscript::fun(&ThisType::GetTasks), "GetTasks");\
AddMemberToScriptFromClass(Tasks);\
AddMemberToScriptFromClass(Opacity);\
AddMemberToScriptFromClass(Width);\
AddMemberToScriptFromClass(Height);\
AddMemberToScriptFromClass(VerticalAlignment);\
AddMemberToScriptFromClass(HorizontalAlignment);\
AddMemberToScriptFromClass(Tag);\
AddMemberToScriptFromClass(Name);\
AddMemberToScriptFromClass(MinWidth);\
AddMemberToScriptFromClass(MinHeight);\
AddMemberToScriptFromClass(MaxWidth);\
AddMemberToScriptFromClass(MaxHeight);\
AddMemberToScriptFromClass(Margin)

class UI_App {
public:
	typedef UI_App ThisType;
	static std::string	ThisTypeName() { return "UI_App"; };

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.eval(R"(
			class AppImpl{};
			attr AppImpl::Think;
			attr AppImpl::SelectFile;
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
				this.SelectFile = fun[](Function todo){
					var& fp = AppFunction("OS_SelectFile");
					try{ todo(); }catch(e){
					try{ todo(fp); }				
					}
					fileSystem.removeFile(fp);
				};
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
					var& out = cweeColor();
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
						var& out = cweeColor();
						out.R = col[0]; out.G = col[1]; out.B = col[2]; out.A = col[3];		
						colMap[key] := out;
					}
					return colMap[key] ;
				};
				App.set_explicit(true);
			}
		)");
	};
};

class UI_FrameworkElement {
public:
	typedef UI_FrameworkElement ThisType;
	static std::string	ThisTypeName() { return "UI_FrameworkElement"; };

	std::vector<chaiscript::Boxed_Value> Tasks;
	int				UniqueName = cweeStr::hash(cweeStr::printf("%i_%i", cweeRandomInt(0, 100000), cweeRandomInt(0, 100000)));
	int				Version = 0;
	int				Update() { 
		return AddTask("Update"); 
	};
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
	
	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		AddFrameworkElementCharacteristicsToScriptFromClass();
	};
};

class UI_Rectangle : public UI_FrameworkElement {
public:
	typedef UI_Rectangle ThisType;
	static  std::string	ThisTypeName() { return "UI_Rectangle"; };

	cweeColor 		Fill;

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Fill);
	};
};
class UI_TextBlock : public UI_FrameworkElement {
public:
	typedef UI_TextBlock ThisType;
	static  std::string	ThisTypeName() { return "UI_TextBlock"; };

	UI_TextBlock() {};
	UI_TextBlock(const cweeStr& text) { 
		Text = text; 
	};

	cweeStr 		TextWrapping = "NoWrap";
	cweeStr 		TextTrimming = "None";
	cweeStr 		TextAlignment = "Center";
	cweeStr			Text;
	cweeStr 		Padding = "0,0,0,0";
	cweeColor 		Foreground;
	double 			FontSize = 18;
	cweeStr 		HorizontalTextAlignment = "Center";

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const cweeStr&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

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
class UI_Image : public UI_FrameworkElement {
public:
	typedef UI_Image ThisType;
	static  std::string	ThisTypeName() { return "UI_Image"; };

	UI_Image() {};
	UI_Image(const cweeStr& imagePath) {
		ImagePath = imagePath;
	};

	cweeStr 		ImagePath;
	cweeStr			Stretch = "UniformToFill";

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const cweeStr&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(ImagePath);
		AddMemberToScriptFromClass(Stretch);
	};
};
class UI_WebView : public UI_FrameworkElement {
public:
	typedef UI_WebView ThisType;
	static  std::string	ThisTypeName() { return "UI_WebView"; };

	UI_WebView() {};
	UI_WebView(const cweeStr& source) {
		Source = source;
	};

	cweeStr 															Source;
	std::map<std::string, chaiscript::Boxed_Value>						ResultQueue;
	int																	NumResultsQueued = 0;
	chaiscript::Boxed_Value												NavigationCompleted;


	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const cweeStr&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

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

class UI_Panel : public UI_FrameworkElement {
public:
	typedef UI_Panel ThisType;
	static std::string	ThisTypeName() { return "UI_Panel"; };

	cweeColor 		Background = cweeColor(0.0f, 0.0f, 0.0f, 0.0f);
	cweeThreadedList<chaiscript::Boxed_Value> Children;
	cweeStr 		Padding = "0,0,0,0";

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Background);
		AddMemberToScriptFromClass(Children);
		AddMemberToScriptFromClass(Padding);
	};
};
class UI_Grid : public UI_Panel {
public:
	typedef UI_Grid ThisType;
	static  std::string	ThisTypeName() { return "UI_Grid"; };

	std::vector<std::string> ColumnDefinitions; // "1*", "5", "Auto"
	std::vector<std::string> RowDefinitions; // "1*", "5", "Auto"
	std::vector<std::string> ChildPositions; // "0,0" or "1,1", etc. 
	double			ColumnSpacing = 0;
	double			RowSpacing = 0;

	void 			AddChild(chaiscript::Boxed_Value const& child, int row, int column, int rowSpan, int columnSpan) {
		this->Children.push_back(child);
		this->ChildPositions.push_back(cweeStr::printf("%i,%i,%i,%i", row, column, rowSpan, columnSpan).c_str());
	};

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		scriptingLanguage.add(chaiscript::base_class<UI_Panel, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Background);
		AddMemberToScriptFromClass(Children);

		AddMemberToScriptFromClass(ColumnDefinitions);
		AddMemberToScriptFromClass(RowDefinitions);
		AddMemberToScriptFromClass(ChildPositions);
		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(ColumnSpacing);
		AddMemberToScriptFromClass(RowSpacing);

		scriptingLanguage.add(chaiscript::fun(&ThisType::AddChild), "AddChild");
	};
};
class UI_StackPanel : public UI_Panel {
public:
	typedef UI_StackPanel ThisType;
	static  std::string	ThisTypeName() { return "UI_StackPanel"; };

	cweeStr 		Orientation = "Vertical";
	double			Spacing = 0;

	void 			AddChild(chaiscript::Boxed_Value const& child) {
		this->Children.Append(child);
	};

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		scriptingLanguage.add(chaiscript::base_class<UI_Panel, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Background);
		AddMemberToScriptFromClass(Children);

		AddMemberToScriptFromClass(Orientation);
		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(Spacing);

		scriptingLanguage.add(chaiscript::fun(&ThisType::AddChild), "AddChild");
	};
};

class UI_Control : public UI_FrameworkElement {
public:
	typedef UI_Control ThisType;
	static std::string	ThisTypeName() { return "UI_Control"; };

	cweeStr 		Padding = "0,0,0,0";
	bool			IsEnabled = true;

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);
	};
};
class UI_Button : public UI_Control {
public:
	typedef UI_Button ThisType;
	static  std::string	ThisTypeName() { return "UI_Button"; };

	chaiscript::Boxed_Value		Content;
	chaiscript::Boxed_Value		Clicked; // Meant to be a lamda function, called when checkbox is unchecked

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);

		AddMemberToScriptFromClass(Content);
		AddMemberToScriptFromClass(Clicked);
	};
};
class UI_CheckBox : public UI_Control {
public:
	typedef UI_CheckBox ThisType;
	static  std::string	ThisTypeName() { return "UI_CheckBox"; };

	bool						IsChecked = true;
	chaiscript::Boxed_Value		Checked; // Meant to be a lamda function, called when checkbox is checked
	chaiscript::Boxed_Value		Unchecked; // Meant to be a lamda function, called when checkbox is unchecked

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);

		AddMemberToScriptFromClass(IsChecked);
		AddMemberToScriptFromClass(Checked);
		AddMemberToScriptFromClass(Unchecked);
	};
};
class UI_Slider : public UI_Control {
public:
	typedef UI_Slider ThisType;
	static  std::string	ThisTypeName() { return "UI_Slider"; };

	double						Value = 0;
	double						SmallChange = 1;
	double						LargeChange = 10;
	double						Minimum = 0;
	double						Maximum = 100;
	double						StepFrequency = 1;
	double						TickFrequency = 1;
	chaiscript::Boxed_Value		ValueChanged; // Meant to be a lamda function, called when slider value changes

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);

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
class UI_ToggleSwitch : public UI_Control {
public:
	typedef UI_ToggleSwitch ThisType;
	static  std::string	ThisTypeName() { return "UI_ToggleSwitch"; };

	bool						IsOn = true;
	chaiscript::Boxed_Value		OnContent; // Meant to be something the UI can draw
	chaiscript::Boxed_Value		OffContent; // Meant to be something the UI can draw
	chaiscript::Boxed_Value		Toggled; // Meant to be a lamda function, called when toggleswitch is toggled

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);

		AddMemberToScriptFromClass(IsOn);
		AddMemberToScriptFromClass(OnContent);
		AddMemberToScriptFromClass(OffContent);
		AddMemberToScriptFromClass(Toggled);
	};
};
class UI_ListView : public UI_Control {
public:
	typedef UI_ListView ThisType;
	static std::string	ThisTypeName() { return "UI_ListView"; };

	UI_ListView() {

	};
	UI_ListView(const std::vector<chaiscript::Boxed_Value>& inputItems) {
		Items = inputItems;
	};

	cweeColor 					Background = cweeColor(0.0f, 0.0f, 0.0f, 0.0f);
	bool						CanReorderItems = false;
	chaiscript::Boxed_Value		Header;
	chaiscript::Boxed_Value		Footer;
	cweeThreadedList<chaiscript::Boxed_Value> Items;

	void 			AddItem(chaiscript::Boxed_Value const& item) {
		this->Items.push_back(item);
	};

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const std::vector<chaiscript::Boxed_Value>&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);

		AddMemberToScriptFromClass(Background);
		AddMemberToScriptFromClass(CanReorderItems);
		AddMemberToScriptFromClass(Header);
		AddMemberToScriptFromClass(Footer);
		AddMemberToScriptFromClass(Items);

		scriptingLanguage.add(chaiscript::fun(&ThisType::AddItem), "AddItem");
	};
};
class UI_TabView : public UI_Control {
public:
	typedef UI_TabView ThisType;
	static std::string	ThisTypeName() { return "UI_TabView"; };

	cweeColor 					Background = cweeColor(0.0f, 0.0f, 0.0f, 0.0f);
	bool						CanReorderTabs = false;
	bool						CanDragTabs = false;
	cweeThreadedList<chaiscript::Boxed_Value> TabItems;
	cweeThreadedList<chaiscript::Boxed_Value> HeaderItems;

	void 			AddTab(chaiscript::Boxed_Value const& tab, chaiscript::Boxed_Value const& header) {
		this->TabItems.push_back(tab);
		this->HeaderItems.push_back(header);
	};

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);

		AddMemberToScriptFromClass(Background);
		AddMemberToScriptFromClass(CanReorderTabs);
		AddMemberToScriptFromClass(CanDragTabs);
		AddMemberToScriptFromClass(TabItems);
		AddMemberToScriptFromClass(HeaderItems);

		scriptingLanguage.add(chaiscript::fun(&ThisType::AddTab), "AddTab");
	};
};
class UI_Expander : public UI_Control {
public:
	typedef UI_Expander ThisType;
	static  std::string	ThisTypeName() { return "UI_Expander"; };

	bool						IsExpanded = true;
	cweeStr						ExpandDirection = "Down";

	chaiscript::Boxed_Value		Header; // Meant to be something the UI can draw
	chaiscript::Boxed_Value		Content; // Meant to be something the UI can draw
	chaiscript::Boxed_Value		Expanding; // Meant to be a lamda function, called when Expander is expanding

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);

		AddMemberToScriptFromClass(IsExpanded);
		AddMemberToScriptFromClass(ExpandDirection);
		AddMemberToScriptFromClass(Header);
		AddMemberToScriptFromClass(Content);
		AddMemberToScriptFromClass(Expanding);
	};
};

class UI_Plot : public UI_Control {
public:
	typedef UI_Plot ThisType;
	static  std::string	ThisTypeName() { return "UI_Plot"; };

	cweeColor 									Background = cweeColor(0.0f, 0.0f, 0.0f, 0.0f);
	cweeColor 									AxisColor = cweeColor(0.0f, 0.0f, 0.0f, 0.0f);
	cweeColor 									FontColor = cweeColor(0.0f, 0.0f, 0.0f, 0.0f);
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

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);

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
			def UI_Plot::AddPlot(values, cweeColor col, string mode) : (values.type_name() == "Pattern" || values.type_name() == "BalancedPattern") {
				this.PlotItems.push_back_ref(values);
				this.PlotColors.push_back_ref(col);
				this.PlotModes.push_back_ref(mode);
				return;
			};
		)");




	};
};

class UI_ScriptingObject : public UI_FrameworkElement {
public:
	typedef UI_ScriptingObject ThisType;
	static  std::string	ThisTypeName() { return "UI_ScriptingObject"; };

	UI_ScriptingObject() {};
	UI_ScriptingObject(const cweeStr& _scriptName) {
		ScriptName = _scriptName;
	};
	
	cweeStr			ScriptName;

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const cweeStr&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(ScriptName);
	};
};

class UI_TextBox : public UI_Control {
public:
	typedef UI_TextBox ThisType;
	static  std::string	ThisTypeName() { return "UI_TextBox"; };

	cweeStr						Text;
	cweeStr						PlaceholderText;
	chaiscript::Boxed_Value		TextChanged; // Meant to be a lamda function, called when checkbox is unchecked

	UI_TextBox() {};
	UI_TextBox(const cweeStr& text) {
		Text = text;
	};

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const cweeStr&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");
		scriptingLanguage.add(chaiscript::base_class<UI_Control, ThisType>());
		scriptingLanguage.add(chaiscript::base_class<UI_FrameworkElement, ThisType>());

		AddFrameworkElementCharacteristicsToScriptFromClass();

		AddMemberToScriptFromClass(Padding);
		AddMemberToScriptFromClass(IsEnabled);

		AddMemberToScriptFromClass(Text);
		AddMemberToScriptFromClass(PlaceholderText);
		AddMemberToScriptFromClass(TextChanged);
	};
};

class UI_Template {
public:
	cweeStr filePath;
	chaiscript::Boxed_Value Content;

public:
	typedef UI_Template ThisType;
	static  std::string	ThisTypeName() { return "UI_Template"; };

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::user_type<ThisType>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType()>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::constructor<ThisType(const ThisType&)>(), ThisTypeName());
		scriptingLanguage.add(chaiscript::fun([](ThisType& a, const ThisType& b) { a = b; return a; }), "=");

		AddMemberToScriptFromClass(filePath);
		AddMemberToScriptFromClass(Content);

		scriptingLanguage.eval(R"(
			def UI_Template(string fileNameOrPath){
				return fun[fileNameOrPath](){
					if (fileNameOrPath.Find(".") >= 0){
						var& out = UI_Template();
						out.filePath = fileNameOrPath;
						out.Content := eval("{"+fileSystem.readFileAsCweeStr(out.filePath)+"}");
						return out;
					}else{
						var& out = UI_Template();
						out.filePath = "${GetDataFolder()}/Templates/${fileNameOrPath}.txt";
						out.Content := eval("{"+fileSystem.readFileAsCweeStr(out.filePath)+"}");
						return out;
					}
				}();
			};
			def `=`(UI_Template out, cweeStr fileContent){		
				return fun[out, fileContent](){
					fileSystem.writeFileFromCweeStr(out.filePath, fileContent);
					return UI_Template(out.filePath);
				}();
			};
			def `=`(out, UI_Template source){
				out = source.Content;
				return out;				
			};
		)");
	}
};

#undef AddMemberToScriptFromClass
#undef AddFuncToScriptFromClass


#endif




