#ifndef TOGGLEUTIL_H
#define TOGGLEUTIL_H

void toggle_AddToRadioGroup(RadioGroup*, Widget);
void toggle_CreateRadioGroup(Widget, Widget);
RadioGroup *toggle_GetRadioGroup(Widget);
void toggle_RemoveFromRadioGroup(Widget);
void toggle_TurnOffRadioSiblings(Widget);


/*
 * Function:
 *	GetRadioGroup
 *
 * Parameters:
 *	w - toggle widget who's radio group we are getting
 *
 * Description:
 *	Gets the radio group associated with a give toggle widget.
 *
 * Returns:
 *	The radio group associated with this toggle group
 */
RadioGroup *toggle_GetRadioGroup(Widget w)
{
    ToggleWidget tw = (ToggleWidget)w;

    if (tw == NULL)
	return (NULL);

    return (tw->toggle.radio_group);
}

/*
 * Function:
 *	CreateRadioGroup
 *
 * Parameters:
 *	w1 - toggle widgets to add to the radio group
 *	w2 - ""
 *
 * Description:
 *	Creates a radio group. give two widgets.
 * 
 * Note:
 *	A pointer to the group is added to each widget's radio_group field.
 */
static void
CreateRadioGroup(Widget w1, Widget w2)
{
    ToggleWidget tw1 = (ToggleWidget)w1;
    ToggleWidget tw2 = (ToggleWidget) w2;

    if (tw1->toggle.radio_group != NULL || tw2->toggle.radio_group != NULL)
	XtAppWarning(XtWidgetToApplicationContext(w1),
		     "Toggle Widget Error - Attempting to create a "
		     "new toggle group, when one already exists.");

    AddToRadioGroup(NULL, w1);
    AddToRadioGroup(GetRadioGroup(w1), w2);
}

/*
 * Function:
 *	AddToRadioGroup
 *
 * Parameters:
 *	group - element of the radio group the we are adding to
 *	w     - new toggle widget to add to the group
 *
 * Description:
 *	Adds a toggle to the radio group.
 */
static void
AddToRadioGroup(RadioGroup *group, Widget w)
{
    ToggleWidget tw = (ToggleWidget)w;
    RadioGroup *local;

    local = (RadioGroup *)XtMalloc(sizeof(RadioGroup));
    local->widget = w;
    tw->toggle.radio_group = local;

    if (group == NULL) {		  /* Creating new group */
	group = local;
	group->next = NULL;
	group->prev = NULL;
	return;
    }
    local->prev = group;	  /* Adding to previous group */
    if ((local->next = group->next) != NULL)
	local->next->prev = local;
    group->next = local;
}

/*
 * Function:
 *	TurnOffRadioSiblings
 *
 * Parameters:
 *	widget - toggle widget
 *
 * Description:
 *	Deactivates all radio siblings.
 */
static void
TurnOffRadioSiblings(Widget w)
{
    RadioGroup *group;
    ToggleWidgetClass cclass = (ToggleWidgetClass)w->core.widget_class;

    if ((group = GetRadioGroup(w)) == NULL)	/* Punt if there is no group */
	return;

    /* Go to the top of the group */
    for (; group->prev != NULL ; group = group->prev)
	;

    while (group != NULL) {
	ToggleWidget local_tog = (ToggleWidget)group->widget;

	if (local_tog->command.set) {
	    cclass->toggle_class.Unset(group->widget, NULL, NULL, NULL);
	    Notify(group->widget, NULL, NULL, NULL);
	}
	group = group->next;
    }
}

/*
 * Function:
 *	RemoveFromRadioGroup
 *
 * Parameters:
 *	w - toggle widget to remove
 *
 * Description:
 *	Removes a toggle from a RadioGroup.
 */
static void
RemoveFromRadioGroup(Widget w)
{
    RadioGroup *group = GetRadioGroup(w);
    if (group != NULL) {
	if (group->prev != NULL)
	    (group->prev)->next = group->next;
	if (group->next != NULL)
	    (group->next)->prev = group->prev;
	XtFree((char *)group);
    }
}


#endif
