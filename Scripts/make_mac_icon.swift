import AppKit
import Foundation

guard CommandLine.arguments.count == 2 else {
    fputs("usage: make_mac_icon.swift destination.png\n", stderr)
    exit(2)
}

let destination = CommandLine.arguments[1]
let size: CGFloat = 1024
let image = NSImage(size: NSSize(width: size, height: size))

func rounded(_ rect: NSRect, _ radius: CGFloat) -> NSBezierPath {
    NSBezierPath(roundedRect: rect, xRadius: radius, yRadius: radius)
}

func line(_ points: [NSPoint], width: CGFloat, color: NSColor) {
    guard let first = points.first else { return }
    let path = NSBezierPath()
    path.move(to: first)
    for point in points.dropFirst() { path.line(to: point) }
    path.lineWidth = width
    path.lineCapStyle = .round
    path.lineJoinStyle = .round
    color.setStroke()
    path.stroke()
}

func label(_ value: String, size: CGFloat, y: CGFloat, color: NSColor, tracking: CGFloat = 0) {
    let paragraph = NSMutableParagraphStyle()
    paragraph.alignment = .center
    let attributes: [NSAttributedString.Key: Any] = [
        .font: NSFont.systemFont(ofSize: size, weight: .black),
        .foregroundColor: color,
        .kern: tracking,
        .paragraphStyle: paragraph
    ]
    (value as NSString).draw(in: NSRect(x: 70, y: y, width: 884, height: size * 1.25), withAttributes: attributes)
}

image.lockFocus()

let outer = NSRect(x: 0, y: 0, width: size, height: size)
let inner = NSRect(x: 42, y: 42, width: 940, height: 940)
let background = NSGradient(colors: [
    NSColor(calibratedRed: 0.020, green: 0.032, blue: 0.070, alpha: 1),
    NSColor(calibratedRed: 0.075, green: 0.025, blue: 0.075, alpha: 1),
    NSColor(calibratedRed: 0.145, green: 0.035, blue: 0.055, alpha: 1)
])!
background.draw(in: rounded(outer, 205), angle: -90)

NSColor(calibratedRed: 0.95, green: 0.16, blue: 0.12, alpha: 1).setStroke()
let border = rounded(inner, 168)
border.lineWidth = 25
border.stroke()

// Peach-colored moon and a small leaf tie the icon to Atlanta without using a city mark.
NSColor(calibratedRed: 1.0, green: 0.51, blue: 0.22, alpha: 1).setFill()
NSBezierPath(ovalIn: NSRect(x: 726, y: 655, width: 154, height: 154)).fill()
NSColor(calibratedRed: 0.20, green: 0.70, blue: 0.38, alpha: 1).setFill()
let leaf = NSBezierPath()
leaf.move(to: NSPoint(x: 794, y: 810))
leaf.curve(to: NSPoint(x: 867, y: 838), controlPoint1: NSPoint(x: 816, y: 850), controlPoint2: NSPoint(x: 850, y: 851))
leaf.curve(to: NSPoint(x: 800, y: 823), controlPoint1: NSPoint(x: 848, y: 812), controlPoint2: NSPoint(x: 822, y: 810))
leaf.fill()

label("THE BATTLE OF", size: 68, y: 770, color: .white, tracking: 5)
label("ATL", size: 294, y: 448, color: .white, tracking: -10)

// A warm Atlanta skyline sits behind the rider and remains recognizable at Finder size.
let skylineColor = NSColor(calibratedRed: 0.97, green: 0.22, blue: 0.14, alpha: 0.72)
skylineColor.setFill()
let buildings: [(CGFloat, CGFloat, CGFloat)] = [
    (116, 136, 115), (205, 92, 158), (275, 112, 124), (360, 86, 192),
    (428, 118, 142), (522, 92, 224), (594, 120, 158), (688, 82, 198),
    (752, 110, 132), (840, 70, 170)
]
for (x, width, height) in buildings {
    NSBezierPath(rect: NSRect(x: x, y: 134, width: width, height: height)).fill()
}
NSBezierPath(rect: NSRect(x: 551, y: 358, width: 34, height: 42)).fill()
let spire = NSBezierPath()
spire.move(to: NSPoint(x: 568, y: 430))
spire.line(to: NSPoint(x: 548, y: 358))
spire.line(to: NSPoint(x: 588, y: 358))
spire.close()
spire.fill()

// The bright trail and bicycle silhouette communicate the actual game at a glance.
line([NSPoint(x: 93, y: 145), NSPoint(x: 930, y: 145)], width: 20,
     color: NSColor(calibratedRed: 1.0, green: 0.54, blue: 0.24, alpha: 1))
let ink = NSColor(calibratedRed: 0.025, green: 0.030, blue: 0.055, alpha: 1)
ink.setStroke()
for x in [326 as CGFloat, 676 as CGFloat] {
    let wheel = NSBezierPath(ovalIn: NSRect(x: x - 82, y: 112, width: 164, height: 164))
    wheel.lineWidth = 23
    wheel.stroke()
}
line([NSPoint(x: 326, y: 194), NSPoint(x: 445, y: 309), NSPoint(x: 548, y: 194),
      NSPoint(x: 326, y: 194), NSPoint(x: 471, y: 194), NSPoint(x: 445, y: 309)], width: 23, color: ink)
line([NSPoint(x: 548, y: 194), NSPoint(x: 618, y: 322), NSPoint(x: 676, y: 194)], width: 23, color: ink)
line([NSPoint(x: 594, y: 322), NSPoint(x: 650, y: 322)], width: 19, color: ink)
line([NSPoint(x: 416, y: 317), NSPoint(x: 474, y: 317)], width: 19, color: ink)
ink.setFill()
NSBezierPath(ovalIn: NSRect(x: 480, y: 327, width: 56, height: 56)).fill()
line([NSPoint(x: 500, y: 337), NSPoint(x: 452, y: 284), NSPoint(x: 548, y: 247)], width: 27, color: ink)
line([NSPoint(x: 452, y: 284), NSPoint(x: 405, y: 214)], width: 27, color: ink)
line([NSPoint(x: 548, y: 247), NSPoint(x: 624, y: 321)], width: 27, color: ink)

image.unlockFocus()

guard let tiff = image.tiffRepresentation,
      let bitmap = NSBitmapImageRep(data: tiff),
      let png = bitmap.representation(using: .png, properties: [:]) else {
    fputs("failed to render icon\n", stderr)
    exit(1)
}
try png.write(to: URL(fileURLWithPath: destination))
