MW.GameController = cc.Class.extend({
    _curScene:null,
    _gameState:MW.GAME_STATE.HOME,
    _isNewGame:true,
    _curLevel:MW.LEVEL.STAGE1,
    _selectLevel:MW.LEVEL.STAGE1,
    init:function () {
        return true;
    },
    setCurScene:function (s) {
        if (this._curScene != s) {
            if (this._curScene !== null) {
                this._curScene.onExit();
            }
            this._curScene = s;
            if (this._curScene) {
                this._curScene.onEnter();
	            cc.director.runScene(s);
            }
        }
    },
    getCurScene:function () {
        return this._curScene;
    },
    runGame:function () {

    },
    newGame:function () {

    },
    option:function () {

    },
    /*about:function () {

    }*/
});

MW.GameController.getInstance = function () {
    cc.assert(this._sharedGame, "Havn't call setSharedGame");
    if (!this._sharedGame) {
        this._sharedGame = new MW.GameController();
        if (this._sharedGame.init()) {
            return this._sharedGame;
        }
    } else {
        return this._sharedGame;
    }
    return null;
};

MW.GameController._sharedGame = null;
