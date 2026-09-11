import AppKit
import Foundation
let destination = CommandLine.arguments[1]
let size = 1024
let image = NSImage(size: NSSize(width: size, height: size))
image.lockFocus()
NSColor(calibratedRed:0.025, green:0.035, blue:0.055, alpha:1).setFill()
NSBezierPath(roundedRect:NSRect(x:0,y:0,width:size,height:size),xRadius:190,yRadius:190).fill()
NSColor(calibratedRed:0.85, green:0.04, blue:0.11, alpha:1).setFill()
NSBezierPath(roundedRect:NSRect(x:62,y:62,width:900,height:900),xRadius:140,yRadius:140).fill()
NSColor(calibratedRed:0.04, green:0.04, blue:0.05, alpha:1).setFill()
NSBezierPath(roundedRect:NSRect(x:87,y:87,width:850,height:850),xRadius:118,yRadius:118).fill()
func text(_ value:String,_ fontSize:CGFloat,_ y:CGFloat,_ color:NSColor) {
 let attrs:[NSAttributedString.Key:Any] = [.font:NSFont.systemFont(ofSize:fontSize,weight:.black),.foregroundColor:color]
 let width=(value as NSString).size(withAttributes:attrs).width
 (value as NSString).draw(at:NSPoint(x:(1024-width)/2,y:y),withAttributes:attrs)
}
text("BATTLE",126,713,.white)
text("FOR THE",77,613,.white)
text("A",410,173,NSColor(calibratedRed:1,green:0.18,blue:0.22,alpha:1))
NSColor(calibratedRed:1,green:0.58,blue:0.31,alpha:1).setFill()
NSBezierPath(ovalIn:NSRect(x:748,y:181,width:92,height:91)).fill()
NSColor(calibratedRed:0.22,green:0.65,blue:0.38,alpha:1).setFill()
NSBezierPath(ovalIn:NSRect(x:791,y:265,width:43,height:19)).fill()
image.unlockFocus()
let rep=NSBitmapImageRep(data:image.tiffRepresentation!)!
try rep.representation(using:.png,properties:[:])!.write(to:URL(fileURLWithPath:destination))
