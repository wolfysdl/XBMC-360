#ifndef CGUILIB_LABEL_H
#define CGUILIB_LABEL_H

#include "GUITextLayout.h"
#include "GUIInfoTypes.h"
#include "GUIFont.h"
#include "Geometry.h"

class CLabelInfo
{
public:
	CLabelInfo()
	{
		font = NULL;
		align = XBFONT_LEFT;
		offsetX = offsetY = 0;
		width = 0;
		angle = 0;
		scrollSpeed = 0;
		scrollSuffix = " | ";
	};

	void UpdateColors()
	{
		textColor.Update();
		shadowColor.Update();
		selectedColor.Update();
		disabledColor.Update();
		focusedColor.Update();
	};

	CGUIInfoColor textColor;
	CGUIInfoColor shadowColor;
	CGUIInfoColor selectedColor;
	CGUIInfoColor disabledColor;
	CGUIInfoColor focusedColor;
	uint32_t align;
	float offsetX;
	float offsetY;
	float width;
	float angle;
	CGUIFont *font;
	int scrollSpeed; 
	CStdString scrollSuffix;
};


// Class for rendering text labels. Handles alignment and rendering of text within a control.
class CGUILabel
{
public:
	// Allowed color categories for labels, as defined by the skin
	enum COLOR { COLOR_TEXT = 0,
				COLOR_SELECTED,
				COLOR_FOCUSED,
				COLOR_DISABLED };

	// Allowed overflow handling techniques for labels, as defined by the skin
	enum OVER_FLOW { OVER_FLOW_TRUNCATE = 0,
					OVER_FLOW_SCROLL,
					OVER_FLOW_WRAP };

	CGUILabel(float posX, float posY, float width, float height, const CLabelInfo& labelInfo, OVER_FLOW overflow = OVER_FLOW_TRUNCATE);
	virtual ~CGUILabel(void);

	// Render the label on screen
	void Render();

	// Set the maximal extent of the label
	// Sets the maximal size and positioning that the label may render in.  Note that <textwidth> can override
	// this, and <textoffsetx> and <textoffsety> may also allow the label to be moved outside this rectangle.
	void SetMaxRect(float x, float y, float w, float h);

	void SetAlign(uint32_t align);

	// Set the text to be displayed in the label
	// Updates the label control and recomputes final position and size
	// param text CStdString to set as this labels text
	void SetText(const CStdString &label);

	// Set the text to be displayed in the label
	// Updates the label control and recomputes final position and size
	// param text CStdStringW to set as this labels text
	void SetTextW(const CStdStringW &label);

	// Set the color to use for the label
	// Sets the color to be used for this label.  Takes effect at the next render
	// param color color to be used for the label
	void SetColor(COLOR color);

	// Set the final layout of the current text
	// Overrides the calculated layout of the current text, forcing a particular size and position
	// param rect CRect containing the extents of the current text
	void SetRenderRect(const CRect &rect) { m_renderRect = rect; };

	// Set whether or not this label control should scroll
	// param scrolling true if this label should scroll.
	void SetScrolling(bool scrolling);

	// Set this label invalid.  Forces an update of the control
	void SetInvalid();

	// Update this labels colors
	void UpdateColors();

	// Returns the precalculated final layout of the current text
	// return CRect containing the extents of the current text
	// SetRenderRect, UpdateRenderRect
	const CRect &GetRenderRect() const { return m_renderRect; };

	// Returns the precalculated full width of the current text, regardless of layout
	// return full width of the current text
	float GetTextWidth() const { return m_textLayout.GetTextWidth(); };

	// Returns the maximal width that this label can render into
	// return Maximal width that this label can render into. Note that this may differ from the
	// amount given in SetMaxRect as offsets and text width overrides have been taken into account.
	float GetMaxWidth() const;

	// Calculates the width of some text
	// param text CStdStringW of text whose width we want
	// return width of the given text
	float CalcTextWidth(const CStdStringW &text) const { return m_textLayout.GetTextWidth(text); };

	const CLabelInfo& GetLabelInfo() const { return m_label; };
	CLabelInfo &GetLabelInfo() { return m_label; };

	/*
	Check a left aligned and right aligned label for overlap and cut the labels off so that no overlap occurs

	If a left-aligned label occupies some of the same space on screen as a right-aligned label, then we may be able to
	correct for this by restricting the width of one or both of them. This routine checks two labels to see whether they
	satisfy this assumption and, if so, adjusts the render rect of both labels so that they no longer do so.  The order
	of the two labels is not important, but we do assume that the left-aligned label is also the left-most on screen, and
	that the right-aligned label is the right most on-screen, so that they overlap due to the fact that one or both of
	the labels are longer than anticipated.  In the following diagram, [R...[R  R] refers to the maximal allowed and
	actual space occupied by the right label.  Similarly, [L   L]...L] refers to the maximal and actual space occupied
	by the left label.  | refers to the central cutting point, i.e. the point that would divide the maximal allowed
	overlap perfectly in two.  There are 3 scenarios to consider:

	cut
	[L       [R...[R  L].|..........L]         R]     left label ends to the left of the cut -> just crop the left label.
	[L       [R.....[R   |      L]..L]         R]     both left and right labels occupy more than the cut allows, so crop both.
	[L       [R..........|.[R   L]..L]         R]     right label ends to the right of the cut -> just crop the right label.

	\param label1 First label to check
	\param label2 Second label to check
	*/
	static void CheckAndCorrectOverlap(CGUILabel &label1, CGUILabel &label2);

protected:
	color_t GetColor() const;

	// Computes the final layout of the text
	// Uses the maximal position and width of the text, as well as the text length
	// and alignment to compute the final render rect of the text.
	void UpdateRenderRect();

private:
	CLabelInfo     m_label;
	CGUITextLayout m_textLayout;

	bool           m_scrolling;
	OVER_FLOW      m_overflowType;
	bool           m_selected;
	CScrollInfo    m_scrollInfo;
	CRect          m_renderRect;   // Actual sizing of text
	CRect          m_maxRect;      // Maximum sizing of text
	bool           m_invalid;      // If true, the label needs recomputing
	COLOR          m_color;        // Color to render text \sa SetColor, GetColor
};

#endif //CGUILIB_LABEL_H