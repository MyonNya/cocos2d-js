cc.game.onStart = function(){
    cc.view.enableRetina(false);
    cc.view.adjustViewPort(true);
    cc.view.setDesignResolutionSize(480,720,cc.ResolutionPolicy.SHOW_ALL);
    cc.view.resizeWithBrowserSize(true);
    cc.director.setProjection(cc.Director.PROJECTION_2D);

    if (cc.sys.isNative) {
        var searchPaths = jsb.fileUtils.getSearchPaths();
        searchPaths.push('script');
        if (cc.sys.os == cc.sys.OS_IOS || cc.sys.os == cc.sys.OS_OSX) {
            searchPaths.push("res");
            searchPaths.push("src");
        }
        jsb.fileUtils.setSearchPaths(searchPaths);
    }
    //load resources
    cc.LoaderScene.preload(g_mainmenu, function () {
        cc.director.runScene(SysMenu.scene());
    }, this);
};

cc.game.run();