import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.3

Button {
    id: mfButton

    contentItem: Text {
        text:                   mfButton.text
        color:                  Material.foreground
        font.bold:              mfButton.font.bold
        font.capitalization:    mfButton.font.capitalization
        font.family:            mfButton.font.family
        font.hintingPreference: mfButton.font.hintingPreference
        font.italic:            mfButton.font.italic
        font.kerning:           mfButton.font.kerning
        font.letterSpacing:     mfButton.font.letterSpacing
        font.overline:          mfButton.font.overline
        font.pointSize:         16
        font.preferShaping:     mfButton.font.preferShaping
        font.strikeout:         mfButton.font.strikeout
        font.underline:         mfButton.font.underline
        font.weight:            mfButton.font.weight
        font.wordSpacing:       mfButton.font.wordSpacing
        horizontalAlignment:    Text.AlignHCenter
        verticalAlignment:      Text.AlignVCenter
        wrapMode:               Text.NoWrap
        fontSizeMode:           Text.Fit
        minimumPointSize:       8
    }
}
