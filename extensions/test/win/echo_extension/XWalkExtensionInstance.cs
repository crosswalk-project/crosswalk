using System;

namespace xwalk
{
  public class XWalkExtensionInstance
  {
    public XWalkExtensionInstance(dynamic native)
    {
      native_ = native;
    }

    public void HandleMessage(String message)
    {
      native_.PostMessageToJS(message);
    }
    public void HandleSyncMessage(String message)
    {
      native_.SendSyncReply(message);
    }

    private dynamic native_;
  }
}
