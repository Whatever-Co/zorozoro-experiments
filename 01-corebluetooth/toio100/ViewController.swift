//
//  ViewController.swift
//  toio100
//
//  Created by Saqoosha on 2020/08/11.
//  Copyright © 2020 Whatever Inc. All rights reserved.
//

import Cocoa


class ViewController: NSViewController {
    
    var bluetooth: BluetoothService!

    override func viewDidLoad() {
        super.viewDidLoad()

        bluetooth = BluetoothService()
        bluetooth.startBluetoothScan()
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }


}

