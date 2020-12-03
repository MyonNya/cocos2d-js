var SettingsLayer = cc.Layer.extend({
    ctor:function(){
        this._super();
        this.init();
    },
    init:function () {
        var sp = new cc.Sprite(res.loading_png);
        sp.anchorX = 0;
        sp.anchorY = 0;
        sp.scale = MW.SCALE;
        this.addChild(sp, 0, 1);

        var cacheImage = cc.textureCache.addImage(res.menuTitle_png);
        var title = new cc.Sprite(cacheImage, cc.rect(0, 0, 134, 39));
        title.x = winSize.width / 2;
        title.y = winSize.height - 120;
        this.addChild(title);


        cc.MenuItemFont.setFontName("Segoe UI");
        cc.MenuItemFont.setFontSize(18);
        var title1 = new cc.MenuItemFont("Âm thanh");
        title1.setEnabled(false);
        title1.setColor(cc.color(MW.FONTCOLOR));

        cc.MenuItemFont.setFontName("Segoe UI");
        cc.MenuItemFont.setFontSize(26);
        var item1 = new cc.MenuItemToggle(
            new cc.MenuItemFont("Bật"),new cc.MenuItemFont("Tắt"));
        item1.setCallback(this.onSoundControl );
        item1.setColor(cc.color(MW.FONTCOLOR));
        var state = MW.SOUND ? 0 : 1;
        item1.setSelectedIndex(state);

        cc.MenuItemFont.setFontName("Segoe UI");
        cc.MenuItemFont.setFontSize(18);
        var title2 = new cc.MenuItemFont("Chế độ");
        title2.setEnabled(false);
        title2.setColor(cc.color(MW.FONTCOLOR));

        cc.MenuItemFont.setFontName("Segoe UI");
        cc.MenuItemFont.setFontSize(26);
        var item2 = new cc.MenuItemToggle(
            new cc.MenuItemFont("Dễ"),
            new cc.MenuItemFont("Bình thường"),
            new cc.MenuItemFont("Khó"));
        item2.setColor(cc.color(MW.FONTCOLOR));
        item2.setCallback( this.onModeControl );


        cc.MenuItemFont.setFontName("Segoe UI");
        cc.MenuItemFont.setFontSize(26);
        var label = new cc.LabelTTF("Quay về", "Arial", 20);
        label.setColor(cc.color(MW.FONTCOLOR));
        var back = new cc.MenuItemLabel(label, this.onBackCallback);
        back.scale = 0.8;

        var menu = new cc.Menu(title1, title2, item1, item2, back);
        menu.alignItemsInColumns(2, 2, 1);
        this.addChild(menu);

        back.y -= 50;

        return true;
    },
    onBackCallback:function (pSender) {
        var scene = new cc.Scene();
        scene.addChild(new SysMenu());
	    cc.director.runScene(new cc.TransitionFade(1.2, scene));
    },
    onSoundControl:function(){
        MW.SOUND = !MW.SOUND;
        var audioEngine = cc.audioEngine;
        if(MW.SOUND){
            audioEngine.playMusic(cc.sys.os == cc.sys.OS_WP8 || cc.sys.os == cc.sys.OS_WINRT ? res.mainMainMusic_wav : res.mainMainMusic_mp3);
        }
        else{
            audioEngine.stopMusic();
	        audioEngine.stopAllEffects();
        }
    },
    onModeControl:function(){
    }
});
